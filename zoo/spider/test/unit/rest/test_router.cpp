//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/rest/router.hpp"
#include "zoo/spider/rest/operation.h"
#include "zoo/spider/rest/pathspec.h"
#include "zoo/spider/messages/message.h"

#include <boost/beast/http/verb.hpp>
#include <boost/describe/class.hpp>

#include <optional>
#include <string>

namespace zoo {
namespace spider {
namespace {

// Minimal error type satisfying IsValidErrorType (only needed to instantiate rest_router).
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

rest_operation get_op(string_view spec, string_view id)
{
	return rest_operation{ .method = verb::get, .path = path_spec{ spec }, .operation_id = id, .summary = {}, .sec = std::nullopt };
}

response_wrapper ok()
{
	return http::response<http::string_body>{ http::status::ok, 11 };
}

} // namespace

// A literal path segment must take precedence over a {parameter} segment regardless of the order in
// which the routes were registered. Here /users/{id} is registered before /users/me, yet a request
// for /users/me must be dispatched to the literal route.
TEST(RestRouter, StaticSegmentBeatsParameterRegardlessOfOrder)
{
	rest_router<test_error> router;

	std::string hit;
	router.add_route(get_op("users/{id}", "by_id"), [&](request&&, url_view&&, path_spec::param_map&&) {
		hit = "by_id";
		return ok();
	});
	router.add_route(get_op("users/me", "me"), [&](request&&, url_view&&, path_spec::param_map&&) {
		hit = "me";
		return ok();
	});

	auto& handler = static_cast<irequest_handler&>(router);

	hit.clear();
	handler.handle_request(request{ verb::get, "/users/me", 11 });
	EXPECT_EQ(hit, "me");

	// A non-literal value still falls through to the parameter route.
	hit.clear();
	handler.handle_request(request{ verb::get, "/users/42", 11 });
	EXPECT_EQ(hit, "by_id");
}

// A percent-encoded slash (%2F) in a request must stay within a single path segment, so it is
// captured whole by a {parameter} rather than splitting the path into extra segments.
TEST(RestRouter, EncodedSlashStaysWithinPathParameter)
{
	rest_router<test_error> router;

	std::string captured;
	router.add_route(get_op("files/{name}", "file"), [&](request&&, url_view&&, path_spec::param_map&& m) {
		captured = std::string{ m.at("name") };
		return ok();
	});

	auto& handler = static_cast<irequest_handler&>(router);
	handler.handle_request(request{ verb::get, "/files/a%2Fb", 11 });

	EXPECT_EQ(captured, "a/b");
}

} // namespace spider
} // namespace zoo
