//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/controller2.hpp"
#include "zoo/spider/listener.h"
#include "zoo/spider/error_response.h"
#include "zoo/spider/file_response.h"
#include "zoo/spider/json_response.h"
#include "zoo/spider/empty_response.h"
#include "zoo/spider/noop_file_event_listener.h"
#include "zoo/spider/aliases.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/describe.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception/all.hpp>

#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <thread>

namespace myns {

using namespace zoo::spider;
using namespace fmt::literals;

BOOST_DEFINE_ENUM_CLASS(Status, available, sold, cancelled)

struct customer final
{
	std::uint64_t       id;
	std::string         name;
	std::vector<Status> statuses{};
};

// Note: structs must be described at namespace scope.
BOOST_DESCRIBE_STRUCT(customer, (), (id, name, statuses))

/// Error handling

struct error final
{
	struct source_location final
	{
		std::optional<std::string_view> file;
		std::optional<std::string_view> function;
		std::optional<int>              line;
	};

	std::string                    message;
	std::optional<int>             error_code;
	std::optional<source_location> location;
};

// Note: structs must be described at namespace scope.
BOOST_DESCRIBE_STRUCT(error, (), (message, error_code, location))
BOOST_DESCRIBE_STRUCT(error::source_location, (), (file, function, line))

class api_exception_handler final : public controller2::exception_handler_base
{
	response handle(const std::exception& e, const request& req) override
	{
		//zlog(err, "{}", boost::diagnostic_information(e)); // very noisy
		zlog(err, "{}", e.what());

		auto err = error{ e.what() };

		if (const auto x = boost::get_error_info<ex_code>(e))
		{
			err.error_code = *x;
		}

		auto status = status::internal_server_error;
		if (const auto x = boost::get_error_info<ex_status>(e))
		{
			status = *x;
		}

		auto loc = error::source_location{};
		if (const auto x = boost::get_error_info<boost::throw_file>(e))
		{
			loc.file = *x;
		}
		if (const auto x = boost::get_error_info<boost::throw_function>(e))
		{
			loc.function = *x;
		}
		if (const auto x = boost::get_error_info<boost::throw_line>(e))
		{
			loc.line = *x;
		}
		if (loc.file || loc.function || loc.line)
		{
			err.location = std::move(loc);
		}

		return json_response::create(req, status, err);
	}
};

class api_controller final : public controller2
{
	struct exception : public exception_base
	{
	};

	std::vector<customer> list_customers(std::string api_key)
	{
		if (api_key != "the_api_key")
		{
			ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "Bad API key" } << exception::status{ status::unauthorized });
		}
		return { customer{ 42, "The Customer Inc", { Status::available, Status::cancelled } } };
	}

	auto get_customer(std::uint64_t                       id,
	                  const std::optional<std::string>&   serial,
	                  const boost::optional<std::string>& api_key,
	                  std::optional<Status>,
	                  const request& req)
	{
		zlog(debug, "{} customer id={}, serial={}, api_key={}", req.method_string(), id, serial, api_key);
		if (!api_key)
		{
			ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "An API key is required" } << exception::status{ status::unauthorized });
		}
		else if (api_key.value() != "the_api_key")
		{
			ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "Bad API key" } << exception::status{ status::unauthorized });
		}
		return customer{ id, "The Customer Inc" };
	}

	auto post_customer(const customer& c, const url_view& url, int16_t)
	{
		zlog(debug, "api post customer {}, number of query parameters={}", c.id, url.params().size());
		return c;
	}

	void noop()
	{
		zlog(debug, "noop");
	}

	void fail(const request& req)
	{
		zlog(debug, "fail, method is {}", req.method_string());
		ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "The error message" } << exception::code{ 42 }
		                                << exception::status{ status::not_implemented });
	}

public:
	explicit api_controller(const std::shared_ptr<request_router2>& router = std::make_shared<request_router2>())
	    : controller2{ router }
	{
		using p = controller2::p;

		this->add_operation(
		    verb::get,                               // HTTP method
		    path_spec{ "api" } / "v1" / "customers", // Path spec
		    &api_controller::list_customers,         // Callback
		    // Descriptors for the callback arguments.
		    // There must be a descriptor for each argument and they must be passed in the same order as the callback arguments.
		    p::header{ "x-api-key" } // Header parameters refer to the header name, e.g. `X-Api-Key: abc12345`
		                             //                                                   ~~~~~~~~~
		);

		this->add_operation(
		    verb::get,                                        // HTTP method
		    path_spec{ "api" } / "v1" / "customers" / "{id}", // Path spec
		    &api_controller::get_customer,                    // Callback
		    // Descriptors for the callback arguments.
		    // There must be a descriptor for each argument and they must be passed in the same order as the callback arguments.
		    p::path{ "id" },          // Path parameters refer to a named sub-expression path spec, i.e. `{id}`
		                              //                                                                   ~~
		    p::query{ "serial" },     // Query parameters refer to the key name, e.g. http://localhost/api/customer/123?serial=123abc
		                              //                                                                                ~~~~~~
		    p::header{ "x-api-key" }, // Header parameters refer to the header name, e.g. `X-Api-Key: abc12345`
		                              //                                                   ~~~~~~~~~
		    p::query{ "status" },
		    p::request{} // Request parameter (const request&)
		);

		this->add_operation(
		    verb::post,                              // HTTP method
		    path_spec{ "api" } / "v1" / "customers", // Path spec
		    &api_controller::post_customer,          // Callback
		    // Descriptors for the callback arguments.
		    // There must be a descriptor for each argument and they must be passed in the same order as the callback arguments.
		    p::json{ "customer" }, // Json parameters are deserialized from the request payload.
		                           // The name ("customer") is used only in logging in case deserialization fails.
		    p::url{},              // Url parameter (const url_view&)
		    p::query{ "u" });

		this->add_operation(
		    verb::get,                          // HTTP method
		    path_spec{ "api" } / "v1" / "fail", // Path spec
		    &api_controller::fail,              // Callback
		    // Descriptors for the callback arguments.
		    // There must be a descriptor for each argument and they must be passed in the same order as the callback arguments.
		    p::request{} // Request parameter (const request&)
		);

		this->add_operation(verb::get,                          // HTTP method
		                    path_spec{ "api" } / "v1" / "noop", // Path spec
		                    &api_controller::noop               // Callback
		);                                                      // No descriptors because callback does not take any arguments.

		this->exception_handler(std::make_shared<api_exception_handler>());
	}

	std::string open_api() const
	{
		const auto jv = this->openapi_spec();
		return boost::json::serialize(jv);
	}
};

} // namespace myns

int main(int argc, char* argv[])
{
	using namespace myns;

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

	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%L [%Y-%m-%d %H:%M:%S.%f Δt=%iμs](%t) %^%v%$ [%s:%#]");

	if (argc == 1)
	{
		std::cout << api_controller{}.open_api() << '\n';
		return 0;
	}

	auto const address = argv[1];
	auto const port    = static_cast<std::uint16_t>(std::atoi(argv[2]));
	auto const threads = std::max<int>(1, std::atoi(argv[3]));

	zlog(info, "application started");

	auto api = api_controller{};

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
