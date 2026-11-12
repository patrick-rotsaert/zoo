//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/http_session.h"
#include "zoo/spider/http_session_settings.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/messages/message.h"
#include "zoo/spider/rest/auth.h"
#include "zoo/spider/rest/bearerauthorization.h"
#include "zoo/spider/rest/controller.hpp"
#include "zoo/spider/rest/openapi.hpp"
#include "zoo/spider/rest/operation.h"
#include "zoo/spider/rest/pathspec.h"
#include "zoo/spider/rest/security.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/describe/class.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <exception>
#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace zoo {
namespace spider {
namespace {

using namespace std::chrono_literals;

struct test_error
{
	std::string message;
	int         error_code{};

	static test_error create(const std::exception& e)
	{
		return test_error{ e.what(), 0 };
	}
	static test_error create(int ec, std::string m)
	{
		return test_error{ std::move(m), ec };
	}
	http::status status() const noexcept
	{
		return static_cast<http::status>(error_code);
	}
};
BOOST_DESCRIBE_STRUCT(test_error, (), (message, error_code))

constexpr std::size_t body_size = std::size_t{ 1 } << 20; // 1 MiB

// PUT /secured needs a bearer token "good"; PUT /open needs nothing. Both answer with the size of the body they got.
class early_reject_controller final : public rest_controller<test_error>
{
public:
	std::atomic<int> auth_calls{ 0 };
	std::atomic<int> handler_calls{ 0 };

	early_reject_controller()
	    : rest_controller{ openapi_settings{} }
	{
		auto bearer = std::make_shared<bearer_authorization>(
		    "BearerAuth",
		    [this](const bearer_authorization& auth, std::string_view token) -> std::expected<auth_data, auth_error> {
			    ++this->auth_calls;
			    if (token == "good")
			    {
				    return auth_data{};
			    }
			    return std::unexpected(auth.make_verification_error("bad token", {}));
		    },
		    "test");

		add_operation(rest_operation{ .method       = verb::put,
		                              .path         = path_spec{ "secured" },
		                              .operation_id = "putSecured",
		                              .summary      = "Secured",
		                              .sec          = security{ { { bearer, {} } } } },
		              &early_reject_controller::put,
		              p::request{});

		add_operation(
		    rest_operation{
		        .method = verb::put, .path = path_spec{ "open" }, .operation_id = "putOpen", .summary = "Open", .sec = security{} },
		    &early_reject_controller::put,
		    p::request{});
	}

	std::size_t put(const request& req)
	{
		++this->handler_calls;
		return req.body().size();
	}
};

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
		// The timeout bounds a test against a server that waits for a body that never comes.
		auto settings       = http_session_settings{};
		settings.body_limit = 16'000'000;
		settings.timeout    = 2s;

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

std::string request_header(std::string_view target, std::string_view authorization)
{
	auto header = std::string{ "PUT " } + std::string{ target } + " HTTP/1.1\r\nHost: test\r\nContent-Type: application/octet-stream\r\n";
	if (!authorization.empty())
	{
		header += "Authorization: " + std::string{ authorization } + "\r\n";
	}
	header += "Content-Length: " + std::to_string(body_size) + "\r\n\r\n";
	return header;
}

} // namespace

// An unauthenticated request is refused as soon as its header is in: the client never sends the body and still gets the
// 401, after which the server closes the connection.
TEST(HttpSessionEarlyReject, UnauthenticatedRequestIsRefusedBeforeTheBody)
{
	early_reject_controller controller;
	test_server             server{ controller.router() };

	net::io_context client_ioc;
	tcp::socket     client{ client_ioc };
	client.connect(tcp::endpoint{ net::ip::make_address("127.0.0.1"), server.port() });

	net::write(client, net::buffer(request_header("/secured", "")));

	beast::flat_buffer                buffer;
	http::response<http::string_body> res;
	beast::error_code                 ec;
	http::read(client, buffer, res, ec);

	ASSERT_FALSE(ec) << ec.message();
	EXPECT_EQ(res.result(), http::status::unauthorized);
	EXPECT_FALSE(res.keep_alive());
	EXPECT_EQ(controller.handler_calls.load(), 0);

	http::response<http::string_body> next;
	http::read(client, buffer, next, ec);
	EXPECT_EQ(ec, http::error::end_of_stream) << ec.message();
}

// An authenticated request reaches its handler once with its whole body, and is authenticated once.
TEST(HttpSessionEarlyReject, AuthenticatedRequestIsVerifiedOnce)
{
	early_reject_controller controller;
	test_server             server{ controller.router() };

	net::io_context client_ioc;
	tcp::socket     client{ client_ioc };
	client.connect(tcp::endpoint{ net::ip::make_address("127.0.0.1"), server.port() });

	net::write(client, net::buffer(request_header("/secured", "Bearer good") + std::string(body_size, 'x')));

	beast::flat_buffer                buffer;
	http::response<http::string_body> res;
	beast::error_code                 ec;
	http::read(client, buffer, res, ec);

	ASSERT_FALSE(ec) << ec.message();
	EXPECT_EQ(res.result(), http::status::ok);
	EXPECT_EQ(res.body(), std::to_string(body_size));
	EXPECT_EQ(controller.handler_calls.load(), 1);
	EXPECT_EQ(controller.auth_calls.load(), 1);
}

// A route without security takes the body without any authentication, and keeps the connection open.
TEST(HttpSessionEarlyReject, UnsecuredRouteIsUnaffected)
{
	early_reject_controller controller;
	test_server             server{ controller.router() };

	net::io_context client_ioc;
	tcp::socket     client{ client_ioc };
	client.connect(tcp::endpoint{ net::ip::make_address("127.0.0.1"), server.port() });

	net::write(client, net::buffer(request_header("/open", "") + std::string(body_size, 'x')));

	beast::flat_buffer                buffer;
	http::response<http::string_body> res;
	beast::error_code                 ec;
	http::read(client, buffer, res, ec);

	ASSERT_FALSE(ec) << ec.message();
	EXPECT_EQ(res.result(), http::status::ok);
	EXPECT_EQ(res.body(), std::to_string(body_size));
	EXPECT_TRUE(res.keep_alive());
	EXPECT_EQ(controller.handler_calls.load(), 1);
	EXPECT_EQ(controller.auth_calls.load(), 0);
}

} // namespace spider
} // namespace zoo
