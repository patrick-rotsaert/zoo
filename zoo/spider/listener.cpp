//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/listener.h"
#include "zoo/spider/http_session.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>

namespace zoo {
namespace spider {

namespace {

void fail(beast::error_code ec, char const* what)
{
	ZOO_LOG(err, "{}: {} ({})", what, ec.message(), ec);
}

} // namespace

// Accepts incoming connections and launches the sessions
class listener_impl final : public std::enable_shared_from_this<listener_impl>
{
	boost::asio::io_context&          ioc_;
	tcp::acceptor                     acceptor_;
	std::shared_ptr<irequest_handler> request_handler_;

public:
	listener_impl(boost::asio::io_context& ioc, const std::shared_ptr<irequest_handler>& request_handler)
	    : ioc_{ ioc }
	    , acceptor_{ boost::asio::make_strand(ioc) }
	    , request_handler_{ request_handler }
	{
	}

	// Start accepting incoming connections
	void run(const tcp::endpoint& endpoint, beast::error_code& ec)
	{
		// Open the acceptor
		this->acceptor_.open(endpoint.protocol(), ec);
		if (ec)
		{
			return fail(ec, "open");
		}

		// Allow address reuse
		this->acceptor_.set_option(net::socket_base::reuse_address(true), ec);
		if (ec)
		{
			return fail(ec, "set_option");
		}

		// Bind to the server address
		this->acceptor_.bind(endpoint, ec);
		if (ec)
		{
			return fail(ec, "bind");
		}

		// Start listening for connections
		this->acceptor_.listen(net::socket_base::max_listen_connections, ec);
		if (ec)
		{
			return fail(ec, "listen");
		}

		// We need to be executing within a strand to perform async operations
		// on the I/O objects in this session. Although not strictly necessary
		// for single-threaded contexts, this example code is written to be
		// thread-safe by default.
		boost::asio::dispatch(this->acceptor_.get_executor(),
		                      beast::bind_front_handler(&listener_impl::do_accept, this->shared_from_this()));
	}

private:
	void do_accept()
	{
		// The new connection gets its own strand
		this->acceptor_.async_accept(net::make_strand(ioc_),
		                             beast::bind_front_handler(&listener_impl::on_accept, this->shared_from_this()));
	}

	void on_accept(beast::error_code ec, tcp::socket socket)
	{
		if (ec)
		{
			fail(ec, "accept");
		}
		else
		{
			ZOO_LOG(trace, "Connection accepted from {}", socket.remote_endpoint());
			// Create the http session and run it
			http_session::run(std::move(socket), this->request_handler_);
		}

		// Accept another connection
		this->do_accept();
	}
};

void listener::run(net::io_context&                         ioc,
                   const std::string_view&                  address,
                   std::uint16_t                            port,
                   const std::shared_ptr<irequest_handler>& request_handler,
                   beast::error_code&                       ec)
{
	const auto addr = net::ip::make_address(address, ec);
	if (ec)
	{
		return fail(ec, "make_address");
	}

	return std::make_shared<listener_impl>(ioc, request_handler)->run(tcp::endpoint{ addr, port }, ec);
}

} // namespace spider
} // namespace zoo
