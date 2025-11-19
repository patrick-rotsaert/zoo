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
#include "zoo/spider/binary_response.h"
#include "zoo/spider/content_container.hpp"
#include "zoo/spider/status_result.hpp"
#include "zoo/spider/aliases.h"
#include "zoo/spider/tag_invoke/uuid.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/misc/rlws.hpp"
#include "zoo/common/misc/uuid.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/describe.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception/all.hpp>

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

#include <fmt/format.h>

#include <thread>

#include "test_png_image.h"

namespace myns {

using namespace zoo::spider;
using namespace fmt::literals;
using namespace std::string_view_literals;

BOOST_DEFINE_ENUM_CLASS(Status, available, sold, cancelled)

struct Customer final
{
	std::uint64_t       id;
	std::string         name;
	std::vector<Status> statuses{};
};

// Note: structs must be described at namespace scope.
BOOST_DESCRIBE_STRUCT(Customer, (), (id, name, statuses))
SPIDER_OAS_REGISTER_TYPE_EXAMPLE(Customer, (Customer{ 1234, "The customer name", { Status::sold } }))

/// Error handling

struct Error final
{
	struct SourceLocation final
	{
		std::optional<std::string_view> file;
		std::optional<std::string_view> function;
		std::optional<int>              line;
	};

	std::string                   message;
	std::optional<int>            error_code;
	std::optional<SourceLocation> location;

	static Error create(const std::exception& e)
	{
		auto err = Error{};

		err.status_ = status::internal_server_error;
		if (const auto x = boost::get_error_info<ex_status>(e))
		{
			err.status_ = *x;
		}

		err.message = e.what();

		if (const auto x = boost::get_error_info<ex_code>(e))
		{
			err.error_code = *x;
		}

		auto loc = Error::SourceLocation{};
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

		return err;
	}

	static Error create(int error_code, std::string message)
	{
		auto err       = Error{};
		err.message    = std::move(message);
		err.error_code = error_code;
		return err;
	}

	http::status status() const noexcept
	{
		return status_;
	}

private:
	http::status status_;
};

// Note: structs must be described at namespace scope.
BOOST_DESCRIBE_STRUCT(Error, (), (message, error_code, location))
BOOST_DESCRIBE_STRUCT(Error::SourceLocation, (), (file, function, line))

static_assert(IsValidErrorType<Error>, "Not a valid error type");

template<typename T>
struct Test
{
	bool succeeded;
	T    data;
};

#define DESCRIBE_TEST_SPECIALIZATION(TYPE, NAME)                                                                                           \
	BOOST_DESCRIBE_STRUCT(Test<TYPE>, (), (succeeded, data))                                                                               \
	using NAME = Test<TYPE>;                                                                                                               \
	SPIDER_OAS_REGISTER_TYPE_NAME(NAME, #NAME)

DESCRIBE_TEST_SPECIALIZATION(std::string, TestString)
SPIDER_OAS_REGISTER_TYPE_EXAMPLE(TestString, (TestString{ true, "The test string" }))

DESCRIBE_TEST_SPECIALIZATION(boost::uuids::uuid, TestUuid)
SPIDER_OAS_REGISTER_TYPE_EXAMPLE(TestUuid, (TestUuid{ true, zoo::conversion::string_to_uuid("969fbc80-ab71-45b2-b34c-d211890823d0") }))

template<http::status Status, typename T>
auto make_status_result(T&& result)
{
	return status_result<Status, T>{ std::move(result) };
}

class api_controller final : public controller2<Error>
{
	struct exception : public exception_base
	{
	};

	auto test()
	{
		// return TestString{ true, "hello" };
		// return status_result<status::accepted, TestString>{ true, "hello" };
		// return make_status_result<status::accepted>(TestString{ true, "hello" });
		return make_status_result<status::accepted>(TestUuid{ true, zoo::uuid::generate() });
	}

	std::variant<status_result<status::ok, TestString>,
	             status_result<status::ok, image_container>, // not used, for OAS demo only
	             status_result<status::not_found, std::string>,
	             status_result<status::not_found, Error> // not used, for OAS demo only
	             >
	test2(bool found)
	{
		if (found)
		{
			return make_status_result<status::ok>(TestString{ true, "hello" });
		}
		else
		{
			return make_status_result<status::not_found>(std::string{ "FAIL" });
		}
	}

	std::vector<Customer> list_customers(std::string api_key)
	{
		if (api_key != "the_api_key")
		{
			ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "Bad API key" } << exception::status{ status::unauthorized });
		}
		return { Customer{ 42, "The Customer Inc", { Status::available, Status::cancelled } } };
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
		return Customer{ id, "The Customer Inc" };
	}

	auto post_customer(const Customer& c, const url_view& url, int16_t)
	{
		zlog(debug, "api post customer {}, number of query parameters={}", c.id, url.params().size());
		// return c;
		return make_status_result<status::created>(c);
	}

	void noop()
	{
		zlog(debug, "noop");
	}

	status_result<status::not_implemented, Error> fail(const request& req, std::optional<bool> useException)
	{
		zlog(debug, "fail, method is {}, use exception is {}", req.method_string(), useException);

		if (useException.value_or(false))
		{
			ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "The exception message" } << exception::code{ 42 }
			                                << exception::status{ status::not_implemented });
		}
		else
		{
			return make_status_result<status::not_implemented>(Error::create(42, "The error message"));
		}
	}

	auto image(const request& req)
	{
		return image_container::create_view("image/png"sv, test_png_image::data());
	}

public:
	explicit api_controller(const std::shared_ptr<request_router2>& router = std::make_shared<request_router2>())
	    : controller2{ router, openapi_settings{ .strip_ns = "myns::", .info_title = "Demo API", .info_version = "1.0" } }
	{
		using p = controller2::p;

		add_operation(operation{ .method       = verb::get,                          // HTTP method
		                         .path         = path_spec{ "api" } / "v1" / "test", // Path spec
		                         .operation_id = "test",
		                         .summary      = "Just a test" },
		              &api_controller::test // Callback
		);
		add_operation(operation{ .method       = verb::get,                           // HTTP method
		                         .path         = path_spec{ "api" } / "v1" / "test2", // Path spec
		                         .operation_id = "test2",
		                         .summary      = "Just another test" },
		              &api_controller::test2, // Callback
		              p::query{ "found" });

		add_operation(operation{ .method       = verb::get,                               // HTTP method
		                         .path         = path_spec{ "api" } / "v1" / "customers", // Path spec
		                         .operation_id = "listCustomers",
		                         .summary      = "Get a list of customers" },
		              &api_controller::list_customers, // Callback
		              // Descriptors for the callback arguments.
		              // There must be a descriptor for each argument and they must be passed in the same order as the callback arguments.
		              p::header{ "x-api-key", "The API Key" } // Header parameters refer to the header name, e.g. `X-Api-Key: abc12345`
		                                                      //                                                   ~~~~~~~~~
		);

		add_operation(operation{ .method       = verb::get,                                        // HTTP method
		                         .path         = path_spec{ "api" } / "v1" / "customers" / "{id}", // Path spec
		                         .operation_id = "getCustomer",
		                         .summary      = "Get a single customer" },
		              &api_controller::get_customer, // Callback
		              // Descriptors for the callback arguments.
		              // There must be a descriptor for each argument and they must be passed in the same order as the callback arguments.
		              p::path{ "id" },      // Path parameters refer to a named sub-expression path spec, i.e. `{id}`
		                                    //                                                                   ~~
		              p::query{ "serial" }, // Query parameters refer to the key name, e.g. http://localhost/api/customer/123?serial=123abc
		                                    //                                                                                ~~~~~~
		              p::header{ "x-api-key" }, // Header parameters refer to the header name, e.g. `X-Api-Key: abc12345`
		                                        //                                                   ~~~~~~~~~
		              p::query{ "status" },
		              p::request{} // Request parameter (const request&)
		);

		add_operation(operation{ .method       = verb::post,                              // HTTP method
		                         .path         = path_spec{ "api" } / "v1" / "customers", // Path spec
		                         .operation_id = "createCustomer",
		                         .summary      = "Create a customer" },
		              &api_controller::post_customer, // Callback
		              // Descriptors for the callback arguments.
		              // There must be a descriptor for each argument and they must be passed in the same order as the callback arguments.
		              p::json{}, // Json parameters are deserialized from the request payload.
		                         // The name ("customer") is used only in logging in case deserialization fails.
		              p::url{},  // Url parameter (const url_view&)
		              p::query{ "u" });

		add_operation(operation{ .method       = verb::get,                          // HTTP method
		                         .path         = path_spec{ "api" } / "v1" / "fail", // Path spec
		                         .operation_id = "fail",
		                         .summary      = "Operation that always fails" },
		              &api_controller::fail, // Callback
		              // Descriptors for the callback arguments.
		              // There must be a descriptor for each argument and they must be passed in the same order as the callback arguments.
		              p::request{}, // Request parameter (const request&)
		              p::query{ "exception" }

		);

		add_operation(operation{ .method       = verb::get,                          // HTTP method
		                         .path         = path_spec{ "api" } / "v1" / "noop", // Path spec
		                         .operation_id = "noop",
		                         .summary      = "An operation that does nothing" },
		              &api_controller::noop // Callback
		);                                  // No descriptors because callback does not take any arguments.

		add_operation(operation{ .method       = verb::get,                           // HTTP method
		                         .path         = path_spec{ "api" } / "v1" / "image", // Path spec
		                         .operation_id = "getImage",
		                         .summary      = "Get the image" },
		              &api_controller::image, // Callback
		              // Descriptors for the callback arguments.
		              // There must be a descriptor for each argument and they must be passed in the same order as the callback arguments.
		              p::request{} // Request parameter (const request&)
		);
	}

	std::string open_api() const
	{
		const auto jv = openapi_spec();
		return boost::json::serialize(jv);
	}
};

} // namespace myns

// Define annotations for class members
// These have to be defined at global namespace (for now?)
SPIDER_OAS_ANNOTATE_MEMBER(myns::Error::message, "The error message")

SPIDER_OAS_ANNOTATE_MEMBER(myns::Customer::id, R"(
	The customer ID.
	This is the primary key.)"_rlws)
SPIDER_OAS_ANNOTATE_MEMBER(myns::Customer::name, "The customer name")

void init_logging_to_stderr()
{
	auto stderr_logger = spdlog::stderr_color_mt("stderr");

	// // 1. Create the specific stderr sink (thread-safe, with colors)
	// auto stderr_sink = std::make_shared<spdlog::sinks::>();

	// // 2. Create a new logger using only this sink
	// //    We name it "stderr_logger" but it will act as the global default.
	// auto stderr_logger = std::make_shared<spdlog::logger>("stderr_logger", stderr_sink);

	// 3. Apply your desired settings to this specific logger
	stderr_logger->set_level(spdlog::level::trace);
	stderr_logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f Δt=%iμs](%t) %^%v%$ [%s:%#]");

	// 4. Set the new logger as the global default.
	//    Any subsequent calls to SPDLOG_... macros will now use this logger,
	//    which only writes to stderr.
	spdlog::set_default_logger(stderr_logger);
}

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

	init_logging_to_stderr();

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
