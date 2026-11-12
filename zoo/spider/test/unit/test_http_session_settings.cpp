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

#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace zoo {
namespace spider {
namespace {

using namespace std::chrono_literals;

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

// Runs a single http_session with default settings, sends `bytes` as they are, and returns the response (or the read error).
post_result run_raw(std::string_view bytes)
{
	net::io_context server_ioc;
	tcp::acceptor   acceptor{ server_ioc, tcp::endpoint{ tcp::v4(), 0 } };
	const auto      port    = acceptor.local_endpoint().port();
	auto            handler = std::make_shared<echo_size_handler>();

	acceptor.async_accept([handler](beast::error_code ec, tcp::socket socket) {
		if (!ec)
		{
			http_session::run(std::move(socket), handler, http_session_settings{});
		}
	});

	std::thread server_thread{ [&server_ioc]() { server_ioc.run(); } };

	net::io_context client_ioc;
	tcp::socket     client{ client_ioc };
	client.connect(tcp::endpoint{ net::ip::make_address("127.0.0.1"), port });

	auto wec = beast::error_code{};
	net::write(client, net::buffer(bytes), wec);

	auto               r = post_result{ {}, {} };
	beast::flat_buffer buffer;
	http::read(client, buffer, r.res, r.ec);

	auto ic = beast::error_code{};
	client.shutdown(tcp::socket::shutdown_both, ic);
	server_ioc.stop();
	server_thread.join();

	return r;
}

constexpr std::size_t large_body_size = std::size_t{ 8 } << 20; // 8 MiB

// Takes longer than the session timeout, then answers with a body far larger than the socket buffers can hold.
class slow_large_handler final : public irequest_handler
{
public:
	response_wrapper handle_request(request&& req) override
	{
		std::this_thread::sleep_for(300ms);
		http::response<http::string_body> res{ http::status::ok, req.version() };
		res.body() = std::string(large_body_size, 'x');
		res.prepare_payload();
		res.keep_alive(req.keep_alive());
		return res;
	}
};

struct slow_read_result
{
	std::string       head;  // the first bytes received
	std::size_t       bytes; // everything received before the connection ended
	beast::error_code ec;    // how it ended
};

// Sends one GET to an http_session with the given settings, then reads nothing for `pause` before reading what is left of the
// connection. Both socket buffers are made small, so the server's write blocks while the client does not read.
slow_read_result run_slow_reader(const http_session_settings& settings, std::chrono::milliseconds pause)
{
	net::io_context server_ioc;
	tcp::acceptor   acceptor{ server_ioc, tcp::endpoint{ tcp::v4(), 0 } };
	const auto      port    = acceptor.local_endpoint().port();
	auto            handler = std::make_shared<slow_large_handler>();

	acceptor.async_accept([handler, settings](beast::error_code ec, tcp::socket socket) {
		if (!ec)
		{
			socket.set_option(tcp::socket::send_buffer_size{ 64 * 1024 }, ec);
			http_session::run(std::move(socket), handler, settings);
		}
	});

	std::thread server_thread{ [&server_ioc]() { server_ioc.run(); } };

	net::io_context client_ioc;
	tcp::socket     client{ client_ioc };
	client.open(tcp::v4());
	client.set_option(tcp::socket::receive_buffer_size{ 8 * 1024 });
	client.connect(tcp::endpoint{ net::ip::make_address("127.0.0.1"), port });

	http::request<http::empty_body> req{ http::verb::get, "/", 11 };
	req.set(http::field::host, "test");
	auto wec = beast::error_code{};
	http::write(client, req, wec);

	std::this_thread::sleep_for(pause);

	auto r  = slow_read_result{ {}, 0u, {} };
	auto in = std::array<char, 64 * 1024>{};
	while (!r.ec)
	{
		const auto n = client.read_some(net::buffer(in), r.ec);
		if (r.head.empty())
		{
			r.head.assign(in.data(), n);
		}
		r.bytes += n;
	}

	auto ic = beast::error_code{};
	client.shutdown(tcp::socket::shutdown_both, ic);
	server_ioc.stop();
	server_thread.join();

	return r;
}

} // namespace

// The default body limit (10 KB) rejects a larger body with 413 and closes the connection.
TEST(HttpSessionSettings, DefaultBodyLimitRejectsLargeBody)
{
	const auto r = run_post(http_session_settings{}, 50u * 1024u);

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::payload_too_large);
	EXPECT_FALSE(r.res.keep_alive());
}

// A header beyond the default header limit (16 KB) is answered with 431.
TEST(HttpSessionSettings, DefaultHeaderLimitRejectsLargeHeader)
{
	const auto r = run_raw("GET / HTTP/1.1\r\nHost: test\r\nX-Big: " + std::string(20u * 1024u, 'x') + "\r\n\r\n");

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::request_header_fields_too_large);
	EXPECT_FALSE(r.res.keep_alive());
}

// Bytes that are not HTTP (here the start of a TLS handshake) are answered with 400.
TEST(HttpSessionSettings, NonHttpBytesGetBadRequest)
{
	const auto r = run_raw(std::string_view{ "\x16\x03\x01\x02\x00\x01\x00\x01\xfc\x03\x03", 11u });

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::bad_request);
	EXPECT_FALSE(r.res.keep_alive());
}

// A request with two different Content-Length headers is answered with 400.
TEST(HttpSessionSettings, ConflictingContentLengthGetsBadRequest)
{
	const auto r = run_raw("POST / HTTP/1.1\r\nHost: test\r\nContent-Length: 1\r\nContent-Length: 2\r\n\r\nxx");

	ASSERT_FALSE(r.ec) << r.ec.message();
	EXPECT_EQ(r.res.result(), http::status::bad_request);
	EXPECT_FALSE(r.res.keep_alive());
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

// A keep-alive response tells the client how long the server keeps an idle connection.
TEST(HttpSessionSettings, KeepAliveResponseAdvertisesTimeout)
{
	const auto byDefault = run_post(http_session_settings{}, 10u);

	ASSERT_FALSE(byDefault.ec) << byDefault.ec.message();
	EXPECT_TRUE(byDefault.res.keep_alive());
	EXPECT_EQ(byDefault.res[http::field::keep_alive], "timeout=30");

	http_session_settings settings;
	settings.timeout = 5s;

	const auto configured = run_post(settings, 10u);

	ASSERT_FALSE(configured.ec) << configured.ec.message();
	EXPECT_EQ(configured.res[http::field::keep_alive], "timeout=5");
}

// A client that takes nothing of the response is closed `timeout` after the write started. The handler runs longer than the
// timeout, so the response is only started if the write gets a deadline of its own rather than the one left from the read.
TEST(HttpSessionSettings, SlowReaderIsClosedAfterTimeout)
{
	http_session_settings settings;
	settings.timeout = 200ms;

	const auto r = run_slow_reader(settings, 1500ms);

	EXPECT_TRUE(std::string_view{ r.head }.starts_with("HTTP/1.1 200")) << r.head.substr(0u, 32u);
	EXPECT_GT(r.bytes, 0u);
	EXPECT_LT(r.bytes, large_body_size);
	EXPECT_TRUE(r.ec == net::error::eof || r.ec == net::error::connection_reset) << r.ec.message();
}

} // namespace spider
} // namespace zoo
