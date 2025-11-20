//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "controller.h"

#include "zoo/spider/listener.h"
#include "zoo/common/logging/logging.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <fmt/format.h>

#include <thread>

namespace demo {

using namespace zoo::spider;
using namespace fmt::literals;

} // namespace demo

void init_logging_to_stderr()
{
	auto stderr_logger = spdlog::stderr_color_mt("stderr");

	stderr_logger->set_level(spdlog::level::trace);
	stderr_logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f Δt=%iμs](%t) %^%v%$ [%s:%#]");

	spdlog::set_default_logger(stderr_logger);
}

int main(int argc, char* argv[])
{
	using namespace demo;

	// Check command line arguments.
	if (argc != 4 && argc != 1)
	{
		fmt::print(stderr,
		           "Usage: {prog} <address> <port> threads>\n"
		           "Example:\n"
		           "    {prog} 0.0.0.0 8080 1\n",
		           "prog"_a = argv[0]);
		return EXIT_FAILURE;
	}

	init_logging_to_stderr();

	if (argc == 1)
	{
		std::cout << Controller{}.openApiSpec() << '\n';
		return EXIT_SUCCESS;
	}

	auto const address = argv[1];
	auto const port    = static_cast<std::uint16_t>(std::atoi(argv[2]));
	auto const threads = std::max<int>(1, std::atoi(argv[3]));

	zlog(info, "application started");

	auto api = Controller{};

	// The io_context is required for all I/O
	auto ioc = boost::asio::io_context{ threads };

	// Create and launch a listening port
	auto ec = error_code{};
	listener::run(ioc, address, port, api.router(), ec);
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
