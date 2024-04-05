//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/listener.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/request_router.h"
#include "zoo/spider/json_response.h"
#include "zoo/spider/file_response.h"
#include "zoo/spider/error_response.h"
#include "zoo/spider/noop_file_event_listener.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/config.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/url/params_view.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/describe.hpp>

#ifdef ZOO_USE_SPDLOG
#include <spdlog/spdlog.h>
#endif

#include <fmt/format.h>

#include <thread>
#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>

using namespace fmt::literals;

class file_event_listener final : public zoo::spider::noop_file_event_listener
{
	void on_close(const error_code& ec, std::uint64_t size, std::uint64_t pos) override
	{
		zlog(info, "on close, size={}, pos={}", size, pos);
	}
};

struct customer final
{
	std::uint64_t id;
	std::string   name;
};

BOOST_DESCRIBE_STRUCT(customer, (), (id, name))

customer get_customer(std::uint64_t id)
{
	return customer{ id, "The Customer Inc" };
}

int main(int argc, char* argv[])
{
	// Check command line arguments.
	if (argc != 5)
	{
		fmt::print(stderr,
		           "Usage: {prog} <address> <port> <doc_root> <threads>\n"
		           "Example:\n"
		           "    {prog} 0.0.0.0 8080 . 1\n",
		           "prog"_a = argv[0]);
		return EXIT_FAILURE;
	}
	auto const address  = argv[1];
	auto const port     = static_cast<std::uint16_t>(std::atoi(argv[2]));
	auto const doc_root = boost::filesystem::path{ argv[3] };
	auto const threads  = std::max<int>(1, std::atoi(argv[4]));

	using namespace zoo::spider;

#ifdef ZOO_USE_SPDLOG
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%L [%Y-%m-%d %H:%M:%S.%f Δt=%iμs](%t) %^%v%$ [%s:%#]");
#endif

	zlog(info, "application started");

	// The io_context is required for all I/O
	auto ioc = boost::asio::io_context{ threads };

	// TODO: explain the routing stuff here

	auto api_router = std::make_shared<request_router>();
	api_router->add_route(
	    verb::get, boost::regex{ R"~(^customer/(\d{1,18})/?$)~" }, [&](auto&& req, auto&& url, auto path, const auto& match) {
		    zlog(debug, "api get customer '{}'", path);
		    assert(match.size() == 1 + 1); // match.size() is the number of marked subexpressions plus 1
		    const auto customer_id = std::stoull(string_view{ match[1].first, match[1].second });
		    zlog(debug, "api get customer {}", customer_id);
		    auto c = get_customer(customer_id);
		    return json_response::create(req, status::ok, std::move(c));
	    });
	api_router->add_json_route<customer>(verb::post,
	                                     boost::regex{ R"~(^customer/?$)~" },
	                                     [&](auto&& req, auto&& url, auto path, const auto& match, boost::json::result<customer>&& data) {
		                                     if (data.has_error())
		                                     {
			                                     zlog(debug, "post customer error: {}", data.error());
			                                     return bad_request::create(req);
		                                     }
		                                     auto&& c = data.value();
		                                     zlog(debug, "api post customer {}", c.id);
		                                     return json_response::create(req, status::ok, std::move(c));
	                                     });

	auto router = std::make_shared<request_router>();
	router->add_route(boost::regex{ R"~(^api/)~" }, api_router);
	router->add_route(
	    { verb::get, verb::head }, boost::regex{ R"~(^files/(.+)/?$)~" }, [&](auto&& req, auto&& url, auto path, const auto& match) {
		    zlog(debug, "files {}", path);
		    assert(match.size() == 1 + 1); // match.size() is the number of marked subexpressions plus 1
		    const auto file_path = string_view{ match[1].first, match[1].second };

		    auto       track = true;
		    const auto pit   = url.params().find("track");
		    if (pit != url.params().end())
		    {
			    if (!boost::conversion::try_lexical_convert<bool>((*pit).value, track))
			    {
				    track = true;
			    }
		    }
		    zlog(debug, "track={}", track);

		    if (track)
		    {
			    return file_response::create(req, doc_root, file_path, std::make_unique<file_event_listener>());
		    }
		    else
		    {
			    return file_response::create(req, doc_root, file_path);
		    }
	    });

	// Create and launch a listening port
	auto ec = error_code{};
	listener::run(ioc, address, port, router, ec);
	if (ec)
	{
		// errors are already logged by listener, no need to repeat that here.
		return EXIT_FAILURE;
	}

	// Capture SIGINT and SIGTERM to perform a clean shutdown
	auto signals = boost::asio::signal_set{ ioc, SIGINT, SIGTERM };
	signals.async_wait([&](const auto&, int sig) {
		// Stop the `io_context`. This will cause `run()`
		// to return immediately, eventually destroying the
		// `io_context` and all of the sockets in it.
		zlog(info, "caught signal {}", sig);
		ioc.stop();
	});

	// Run the I/O service on the requested number of threads
	auto v = std::vector<std::thread>{};
	v.reserve(threads - 1);
	for (auto i = threads - 1; i > 0; --i)
	{
		v.emplace_back([&ioc] { ioc.run(); });
	}
	ioc.run();

	// (If we get here, it means we got a SIGINT or SIGTERM)

	// Block until all the threads exit
	for (auto& t : v)
	{
		t.join();
	}

	return EXIT_SUCCESS;
}
