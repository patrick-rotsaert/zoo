//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/http_session.h"
#include "zoo/spider/irequest_handler.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <thread>

namespace zoo {
namespace spider {
namespace {

// Returns a large body so a single response cannot be written in one shot: with the client not
// draining, the first response's socket write suspends, which is exactly the window in which a
// buggy write-queue would start a second, concurrent async_write on the same stream.
class big_body_handler final : public irequest_handler
{
	std::string body_;

public:
	explicit big_body_handler(std::size_t size)
	    : body_(size, 'x')
	{
	}

	response_wrapper handle_request(request&& req) override
	{
		http::response<http::string_body> res{ http::status::ok, req.version() };
		res.set(http::field::content_type, "application/octet-stream");
		res.body() = this->body_;
		res.prepare_payload();
		res.keep_alive(req.keep_alive()); // keep the connection alive so the client can pipeline
		return res;
	}
};

} // namespace

// Regression test for the http_session write queue.
//
// A single HTTP/1.1 connection with pipelined requests must never cause two concurrent
// beast::async_write operations on the same stream. With the original (buggy) queue the message was
// removed from the queue when the write STARTED, so a pipelined read completing while a large write
// was still in flight would start a second write -> beast's pending_guard assertion fires in Debug
// ("only one pending write"), or the response byte stream is corrupted in Release. With the fix the
// message stays queued until on_write(), so at most one write is ever in flight and every response
// arrives intact and in order.
TEST(HttpSessionPipelining, PipelinedRequestsDoNotOverlapWrites)
{
	constexpr std::size_t body_size     = std::size_t{ 1 } << 20; // 1 MiB, larger than socket buffers
	constexpr int         request_count = 8;

	net::io_context server_ioc;
	tcp::acceptor   acceptor{ server_ioc, tcp::endpoint{ tcp::v4(), 0 } };
	const auto      port    = acceptor.local_endpoint().port();
	auto            handler = std::make_shared<big_body_handler>(body_size);

	acceptor.async_accept([handler](beast::error_code ec, tcp::socket socket) {
		if (!ec)
		{
			http_session::run(std::move(socket), handler);
		}
	});

	std::thread server_thread{ [&server_ioc]() { server_ioc.run(); } };

	// --- client: fully synchronous ---
	net::io_context client_ioc;
	tcp::socket     client{ client_ioc };
	client.connect(tcp::endpoint{ net::ip::make_address("127.0.0.1"), port });

	// Send every request up front, before reading any response. This guarantees that later requests
	// are already buffered on the server while the first (large) response is still being written.
	std::string requests;
	for (int i = 0; i < request_count; ++i)
	{
		requests += "GET /" + std::to_string(i) + " HTTP/1.1\r\nHost: test\r\n\r\n";
	}
	net::write(client, net::buffer(requests));

	// Read and validate every response. Corruption from overlapping writes shows up here as a read
	// error or a truncated body; an overlapping write in a Debug build aborts the server first.
	beast::flat_buffer buffer;
	for (int i = 0; i < request_count; ++i)
	{
		http::response<http::string_body> res;
		beast::error_code                 ec;
		http::read(client, buffer, res, ec);

		ASSERT_FALSE(ec) << "failed reading response " << i << ": " << ec.message();
		EXPECT_EQ(res.result(), http::status::ok) << "response " << i;
		EXPECT_EQ(res.body().size(), body_size) << "response " << i << " body truncated/corrupted";
	}

	auto ec = beast::error_code{};
	client.shutdown(tcp::socket::shutdown_both, ec);
	client.close(ec);

	server_ioc.stop();
	server_thread.join();
}

} // namespace spider
} // namespace zoo
