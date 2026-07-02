//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/http_session.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/messages/error_response.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/message_generator.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/optional/optional.hpp>

#include <vector>

namespace zoo {
namespace spider {

using namespace std::chrono_literals;

namespace {

void fail(beast::error_code ec, char const* what)
{
	ZOO_LOG(err, "{}: {} ({})", what, ec.message(), ec);
}

// Non-throwing remote_endpoint(): the throwing overload would throw (and escape the accept handler
// that constructs the session) if the peer reset the connection between accept and here.
tcp::socket::endpoint_type safe_remote_endpoint(const tcp::socket& socket)
{
	auto ec = beast::error_code{};
	return socket.remote_endpoint(ec);
}

} // namespace

// Handles an HTTP server connection
class http_session_impl final : public std::enable_shared_from_this<http_session_impl>
{
	static constexpr std::size_t queue_limit = 8; // max responses

	tcp::socket::endpoint_type        endpoint_;
	beast::tcp_stream                 stream_;
	std::shared_ptr<irequest_handler> request_handler_;
	beast::flat_buffer                buffer_;
	std::vector<message_generator>    response_queue_;

	// The parser is stored in an optional container so we can
	// construct it from scratch it at the beginning of each new message.
	boost::optional<http::request_parser<http::string_body>> parser_;

public:
	// Take ownership of the socket
	http_session_impl(tcp::socket&& socket, const std::shared_ptr<irequest_handler>& request_handler)
	    : endpoint_{ safe_remote_endpoint(socket) }
	    , stream_{ std::move(socket) }
	    , request_handler_{ request_handler }
	    , buffer_{}
	    , response_queue_{}
	    , parser_{}
	{
		static_assert(queue_limit > 0, "queue limit must be positive");
		this->response_queue_.reserve(queue_limit);
	}

	~http_session_impl()
	{
		ZOO_LOG(trace, "Ended session with {}", this->endpoint_);
	}

	// Start the session
	void run()
	{
		// We need to be executing within a strand to perform async operations
		// on the I/O objects in this session. Although not strictly necessary
		// for single-threaded contexts, this example code is written to be
		// thread-safe by default.
		net::dispatch(this->stream_.get_executor(), beast::bind_front_handler(&http_session_impl::do_read, this->shared_from_this()));
	}

private:
	void do_read()
	{
		// Construct a new parser for each message
		this->parser_.emplace();

		// Apply a reasonable limit to the allowed size
		// of the body in bytes to prevent abuse.
		this->parser_->body_limit(10000);

		// Set the timeout.
		this->stream_.expires_after(30s);

		// Read a request using the parser-oriented interface
		http::async_read(
		    this->stream_, this->buffer_, *this->parser_, beast::bind_front_handler(&http_session_impl::on_read, this->shared_from_this()));
	}

	void on_read(beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		// Client closed the connection or read timed out
		if (ec == http::error::end_of_stream || ec == beast::error::timeout)
		{
			ZOO_LOG(debug, "read: {}", ec.message());
			return this->do_close();
		}

		if (ec)
		{
			return fail(ec, "read");
		}

#if 0
		// See if it is a WebSocket Upgrade
		if (websocket::is_upgrade(parser_->get()))
		{
			// Create a websocket session, transferring ownership
			// of both the socket and the HTTP request.
			std::make_shared<websocket_session>(stream_.release_socket())->do_accept(parser_->release());
			return;
		}
#endif

		// Send the response. This runs inside an Asio completion handler, so an exception escaping
		// the request handler would propagate into io_context::run() on a worker thread and call
		// std::terminate. Translate any handler exception into a 500 response instead.
		response_wrapper response = [&]() -> response_wrapper {
			try
			{
				return this->request_handler_->handle_request(this->parser_->release());
			}
			catch (const std::exception& e)
			{
				ZOO_LOG(err, "Unhandled exception in request handler: {}", e.what());
				return internal_server_error::create();
			}
			catch (...)
			{
				ZOO_LOG(err, "Unhandled exception in request handler");
				return internal_server_error::create();
			}
		}();

		this->queue_write(std::move(response));

		// If we aren't at the queue limit, try to pipeline another request
		if (this->response_queue_.size() < queue_limit)
		{
			this->do_read();
		}
	}

	void queue_write(message_generator&& response)
	{
		// Allocate and store the work
		this->response_queue_.push_back(std::move(response));

		// If there was no previous work, start the write loop
		if (this->response_queue_.size() == 1u)
		{
			this->do_write();
		}
	}

	// Called to start/continue the write-loop. Should not be called when a write is already in
	// progress. The message being written is kept at the front of the queue until on_write() runs,
	// so response_queue_.size() reflects the in-flight write. This is what prevents queue_write()
	// from starting a second, concurrent async_write on the same stream (which is undefined
	// behaviour). Mirrors the Boost.Beast advanced_server example.
	void do_write()
	{
		if (!this->response_queue_.empty())
		{
			const auto keep_alive = this->response_queue_.front().keep_alive();

			beast::async_write(this->stream_,
			                   std::move(this->response_queue_.front()),
			                   beast::bind_front_handler(&http_session_impl::on_write, this->shared_from_this(), keep_alive));
		}
	}

	void on_write(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
		{
			return fail(ec, "write");
		}

		if (!keep_alive)
		{
			// This means we should close the connection, usually because
			// the response indicated the "Connection: close" semantic.
			return this->do_close();
		}

		// If the queue was full the read loop was paused (see on_read); resume it now that a slot
		// is about to free up. Checked before erasing, while size() still counts the completed write.
		if (this->response_queue_.size() == queue_limit)
		{
			this->do_read();
		}

		// Remove the completed response and continue the write loop with the next one, if any.
		this->response_queue_.erase(this->response_queue_.begin());

		this->do_write();
	}

	void do_close()
	{
		// Send a TCP shutdown
		auto ec = beast::error_code{};
		this->stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

		// At this point the connection is closed gracefully
	}
};

void http_session::run(tcp::socket&& socket, const std::shared_ptr<irequest_handler>& request_handler)
{
	std::make_shared<http_session_impl>(std::move(socket), request_handler)->run();
}

} // namespace spider
} // namespace zoo
