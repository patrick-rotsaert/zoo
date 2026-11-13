//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/aliases.h"
#include "zoo/spider/http_session.h"
#include "zoo/spider/http_session_settings.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/messages/message.h"
#include "zoo/spider/messages/response_wrapper.hpp"
#include "zoo/spider/rest/auth.h"

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <fmt/format.h>

#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace zoo {
namespace spider {
namespace {

using namespace std::chrono_literals;

// Serves one connection with http_session, on its own thread.
class test_server final
{
	net::io_context ioc_;
	tcp::acceptor   acceptor_;
	std::thread     thread_;

public:
	explicit test_server(std::shared_ptr<irequest_handler> handler)
	    : ioc_{}
	    , acceptor_{ ioc_, tcp::endpoint{ tcp::v4(), 0 } }
	    , thread_{}
	{
		// The timeout bounds a test against a server that waits for bytes that never come.
		auto settings    = http_session_settings{};
		settings.timeout = 2s;

		this->acceptor_.async_accept([handler, settings](beast::error_code ec, tcp::socket socket) {
			if (!ec)
			{
				http_session::run(std::move(socket), handler, settings);
			}
		});
		this->thread_ = std::thread{ [this]() { this->ioc_.run(); } };
	}

	~test_server()
	{
		this->ioc_.stop();
		this->thread_.join();
	}

	test_server(const test_server&)            = delete;
	test_server& operator=(const test_server&) = delete;

	unsigned short port() const
	{
		return this->acceptor_.local_endpoint().port();
	}
};

struct raw_result
{
	beast::error_code                 ec;
	http::response<http::string_body> res;
};

// Sends `bytes` as they are and returns the response (or the read error).
raw_result send_raw(unsigned short port, std::string_view bytes)
{
	net::io_context client_ioc;
	tcp::socket     client{ client_ioc };
	client.connect(tcp::endpoint{ net::ip::make_address("127.0.0.1"), port });

	auto wec = beast::error_code{};
	net::write(client, net::buffer(bytes), wec);

	auto               r = raw_result{ {}, {} };
	beast::flat_buffer buffer;
	http::read(client, buffer, r.res, r.ec);

	auto ic = beast::error_code{};
	client.shutdown(tcp::socket::shutdown_both, ic);

	return r;
}

// Throws where it is told to, and answers the session's errors with its own plain-text response when custom_errors is set.
class scripted_handler final : public irequest_handler
{
public:
	bool throw_in_header{ false };
	bool throw_in_request{ false };
	bool custom_errors{ false };
	bool throw_in_error{ false };

	std::optional<response_wrapper> handle_request_header(const http::request_header<>&, std::optional<auth_map>&) override
	{
		if (this->throw_in_header)
		{
			throw std::runtime_error{ "header handler failed" };
		}
		return std::nullopt;
	}

	response_wrapper handle_request(request&& req) override
	{
		if (this->throw_in_request)
		{
			throw std::runtime_error{ "request handler failed" };
		}
		http::response<http::string_body> res{ http::status::ok, req.version() };
		res.prepare_payload();
		res.keep_alive(req.keep_alive());
		return res;
	}

	response_wrapper make_error_response(http::status status, std::string_view message) override
	{
		if (this->throw_in_error)
		{
			throw std::runtime_error{ "error response failed" };
		}
		if (!this->custom_errors)
		{
			return irequest_handler::make_error_response(status, message);
		}
		http::response<http::string_body> res{ status, 11 };
		res.set(http::field::content_type, "text/plain");
		res.body() = fmt::format("{} {}", static_cast<int>(status), message);
		res.prepare_payload();
		return res;
	}
};

// A header larger than the default header limit, so the parser rejects it with 431.
std::string big_request()
{
	return "GET / HTTP/1.1\r\nHost: test\r\nX-Big: " + std::string(20u * 1024u, 'x') + "\r\n\r\n";
}

constexpr std::string_view plain_request{ "GET / HTTP/1.1\r\nHost: test\r\n\r\n" };

} // namespace

TEST(HttpSessionErrorResponse, ParseErrorIsAnsweredByTheHandler)
{
	auto handler           = std::make_shared<scripted_handler>();
	handler->custom_errors = true;
	test_server server{ handler };

	const auto r = send_raw(server.port(), big_request());

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::request_header_fields_too_large);
	EXPECT_TRUE(r.res.body().starts_with("431 Failed to parse the request: ")) << r.res.body();
	EXPECT_FALSE(r.res.keep_alive());
}

TEST(HttpSessionErrorResponse, ExceptionInTheRequestHandlerIsAnsweredByTheHandler)
{
	auto handler              = std::make_shared<scripted_handler>();
	handler->custom_errors    = true;
	handler->throw_in_request = true;
	test_server server{ handler };

	const auto r = send_raw(server.port(), plain_request);

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::internal_server_error);
	EXPECT_EQ(r.res.body(), "500 Internal server error");
	EXPECT_TRUE(r.res.keep_alive());
}

TEST(HttpSessionErrorResponse, ExceptionInTheHeaderHandlerIsAnsweredByTheHandler)
{
	auto handler             = std::make_shared<scripted_handler>();
	handler->custom_errors   = true;
	handler->throw_in_header = true;
	test_server server{ handler };

	const auto r = send_raw(server.port(), plain_request);

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::internal_server_error);
	EXPECT_EQ(r.res.body(), "500 Internal server error");
	EXPECT_FALSE(r.res.keep_alive());
}

TEST(HttpSessionErrorResponse, HandlerWithoutErrorResponsesGetsTheStockPage)
{
	auto        handler = std::make_shared<scripted_handler>();
	test_server server{ handler };

	const auto r = send_raw(server.port(), big_request());

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::request_header_fields_too_large);
	EXPECT_EQ(r.res[http::field::content_type], "text/html");
}

TEST(HttpSessionErrorResponse, HandlerThatFailsToMakeTheErrorResponseGetsTheStockPage)
{
	auto handler            = std::make_shared<scripted_handler>();
	handler->throw_in_error = true;
	test_server server{ handler };

	const auto r = send_raw(server.port(), big_request());

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::request_header_fields_too_large);
	EXPECT_EQ(r.res[http::field::content_type], "text/html");
}

} // namespace spider
} // namespace zoo
