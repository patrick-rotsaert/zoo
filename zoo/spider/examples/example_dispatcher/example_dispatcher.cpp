//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/dispatcher.hpp"
#include "zoo/spider/listener.h"
#include "zoo/spider/error_response.h"
#include "zoo/spider/file_response.h"
#include "zoo/spider/json_response.h"
#include "zoo/spider/noop_file_event_listener.h"
#include "zoo/spider/aliases.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/config.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/describe.hpp>
#include <boost/throw_exception.hpp>

#ifdef ZOO_USE_SPDLOG
#include <spdlog/spdlog.h>
#endif

#include <fmt/format.h>

#include <thread>

using namespace zoo::spider;
using namespace fmt::literals;

// Note: structs must be described at namespace scope.
struct customer final
{
	std::uint64_t id;
	std::string   name;
};

BOOST_DESCRIBE_STRUCT(customer, (), (id, name))

class api_server final
{
	message_generator get_customer(request&& req, std::uint64_t id, const std::optional<std::string>& serial)
	{
		zlog(debug, "get customer id={}, serial={}", id, serial);
		return json_response::create(req, status::ok, customer{ id, "The Customer Inc" });
	}

	message_generator post_customer(request&& req, const customer& c)
	{
		zlog(debug, "api post customer {}", c.id);
		return json_response::create(req, status::ok, c);
	}

public:
	explicit api_server(const std::shared_ptr<request_router>& router)
	{
		dispatcher::register_action(router,
		                            { verb::get },
		                            boost::regex{ R"~(^customer/(?<id>\d{1,18})/?$)~" },
		                            this,
		                            &api_server::get_customer,
		                            { path_parameter{ "id" }, query_parameter{ "serial" } });

		dispatcher::register_action(
		    router, { verb::post }, boost::regex{ R"~(^customer/?$)~" }, this, &api_server::post_customer, { json_body{ "customer" } });
	}
};

class file_server final
{
	boost::filesystem::path doc_root_;

	class file_event_listener final : public noop_file_event_listener
	{
		void on_close(const error_code& ec, std::uint64_t size, std::uint64_t pos) override
		{
			zlog(info, "on close, size={}, pos={}", size, pos);
		}
	};

	message_generator get_file(request&& req, const std::string& path, boost::optional<bool> track)
	{
		zlog(debug, "get file path={}, track={}", path, track);
		if (track.value_or(true))
		{
			return file_response::create(req, this->doc_root_, path, std::make_unique<file_event_listener>());
		}
		else
		{
			return file_response::create(req, this->doc_root_, path);
		}
	}

public:
	explicit file_server(const std::shared_ptr<request_router>& router, const boost::filesystem::path& doc_root)
	    : doc_root_{ doc_root }
	{
		dispatcher::register_action(router,
		                            { verb::get },
		                            boost::regex{ R"~(^(?<path>.+)/?$)~" },
		                            this,
		                            &file_server::get_file,
		                            { path_parameter{ "path" }, query_parameter{ "track" } });
	}
};

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

#ifdef ZOO_USE_SPDLOG
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%L [%Y-%m-%d %H:%M:%S.%f Δt=%iμs](%t) %^%v%$ [%s:%#]");
#endif

	zlog(info, "application started");

	auto api_router = std::make_shared<request_router>();
	auto api        = api_server{ api_router };

	auto files_router = std::make_shared<request_router>();
	auto files        = file_server{ files_router, doc_root };

	auto router = std::make_shared<request_router>();
	router->add_route(boost::regex{ R"~(^api/)~" }, api_router);
	router->add_route(boost::regex{ R"~(^files/)~" }, files_router);

	// The io_context is required for all I/O
	auto ioc = boost::asio::io_context{ threads };

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
