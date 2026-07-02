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

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <thread>

namespace zoo {
namespace spider {
namespace {

// Echoes the received request body size back in the response body.
class echo_size_handler final : public irequest_handler
{
public:
	response_wrapper handle_request(request&& req) override
	{
		http::response<http::string_body> res{ http::status::ok, req.version() };
		res.body() = std::to_string(req.body().size());
		res.prepare_payload();
		res.keep_alive(req.keep_alive());
		return res;
	}
};

struct post_result
{
	beast::error_code                 ec;
	http::response<http::string_body> res;
};

// Runs a single http_session with the given settings, sends one POST with a body of body_size, and
// returns the response (or the read error if the server rejected/closed the connection).
post_result run_post(const http_session_settings& settings, std::size_t body_size)
{
	net::io_context server_ioc;
	tcp::acceptor   acceptor{ server_ioc, tcp::endpoint{ tcp::v4(), 0 } };
	const auto      port    = acceptor.local_endpoint().port();
	auto            handler = std::make_shared<echo_size_handler>();

	acceptor.async_accept([handler, settings](beast::error_code ec, tcp::socket socket) {
		if (!ec)
		{
			http_session::run(std::move(socket), handler, settings);
		}
	});

	std::thread server_thread{ [&server_ioc]() { server_ioc.run(); } };

	net::io_context client_ioc;
	tcp::socket     client{ client_ioc };
	client.connect(tcp::endpoint{ net::ip::make_address("127.0.0.1"), port });

	http::request<http::string_body> req{ http::verb::post, "/", 11 };
	req.set(http::field::host, "test");
	req.body() = std::string(body_size, 'x');
	req.prepare_payload();
	auto wec = beast::error_code{};
	http::write(client, req, wec);

	post_result        r;
	beast::flat_buffer buffer;
	http::read(client, buffer, r.res, r.ec);

	auto ic = beast::error_code{};
	client.shutdown(tcp::socket::shutdown_both, ic);
	server_ioc.stop();
	server_thread.join();

	return r;
}

} // namespace

// The default body limit (10 KB) rejects a larger body.
TEST(HttpSessionSettings, DefaultBodyLimitRejectsLargeBody)
{
	const auto r = run_post(http_session_settings{}, 50u * 1024u);
	EXPECT_TRUE(r.ec || r.res.result() != http::status::ok);
}

// Raising the body limit allows the larger body through.
TEST(HttpSessionSettings, RaisedBodyLimitAcceptsLargeBody)
{
	http_session_settings settings;
	settings.body_limit = 1u << 20; // 1 MiB

	const auto r = run_post(settings, 50u * 1024u);

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::ok);
	EXPECT_EQ(r.res.body(), std::to_string(50u * 1024u));
}

} // namespace spider
} // namespace zoo
