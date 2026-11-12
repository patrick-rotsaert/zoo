//
// Copyright (C) 2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/rest/openapi.hpp"
#include "zoo/spider/rest/operation.h"
#include "zoo/spider/rest/parameters.h"
#include "zoo/spider/rest/pathspec.h"
#include "zoo/spider/aliases.h"

#include <boost/describe/class.hpp>
#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/string.hpp>
#include <boost/json/value.hpp>
#include <boost/optional/optional.hpp>

#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace zoo {
namespace spider {
namespace {

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

struct nested_body
{
	int value{};

	static std::string_view type_name()
	{
		return "nested_body";
	}
};
BOOST_DESCRIBE_STRUCT(nested_body, (), (value))

struct optional_body
{
	int                          mandatory{};
	std::optional<int>           optional_int;
	boost::optional<nested_body> optional_nested;
	std::shared_ptr<nested_body> shared_nested;

	static std::string_view type_name()
	{
		return "optional_body";
	}
};
BOOST_DESCRIBE_STRUCT(optional_body, (), (mandatory, optional_int, optional_nested, shared_nested))

struct test_operations
{
	void post(const optional_body&, const boost::optional<int>&)
	{
	}
};

boost::json::object generate_spec()
{
	using p = parameters::p;

	openapi<test_error> oas{ openapi_settings{} };
	oas.add_operation(
	    rest_operation{ .method = verb::post, .path = path_spec{ "items" }, .operation_id = "post", .summary = {}, .sec = std::nullopt },
	    &test_operations::post,
	    p::json{},
	    p::query{ "limit" });
	return oas.spec();
}

} // namespace

TEST(OpenApi, OptionalPropertiesAreNullable)
{
	const auto  spec   = generate_spec();
	const auto& schema = spec.at("components").at("schemas").at("optional_body").as_object();
	const auto& props  = schema.at("properties").as_object();

	EXPECT_FALSE(props.at("mandatory").as_object().contains("nullable"));

	const auto& optional_int = props.at("optional_int").as_object();
	EXPECT_EQ(optional_int.at("type"), "integer");
	EXPECT_EQ(optional_int.at("nullable"), true);

	for (const auto name : { "optional_nested", "shared_nested" })
	{
		const auto& prop = props.at(name).as_object();
		EXPECT_FALSE(prop.contains("$ref")) << name;
		EXPECT_EQ(prop.at("nullable"), true) << name;
		const auto& all_of = prop.at("allOf").as_array();
		ASSERT_EQ(all_of.size(), 1u) << name;
		EXPECT_EQ(all_of.at(0).at("$ref"), "#/components/schemas/nested_body") << name;
	}

	EXPECT_EQ(schema.at("required"), (boost::json::array{ "mandatory" }));
}

TEST(OpenApi, OptionalParameterIsNullable)
{
	const auto  spec      = generate_spec();
	const auto& parameter = spec.at("paths").at("/items").at("post").at("parameters").at(0).as_object();

	EXPECT_EQ(parameter.at("name"), "limit");
	EXPECT_EQ(parameter.at("required"), false);
	EXPECT_EQ(parameter.at("schema").at("nullable"), true);
}

} // namespace spider
} // namespace zoo
