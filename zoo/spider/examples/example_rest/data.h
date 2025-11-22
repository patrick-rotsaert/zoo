//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/rest/status_result.hpp"
#include "zoo/spider/rest/annotation.hpp"
#include "zoo/spider/aliases.h"
#include "zoo/common/misc/rlws.hpp"

#include <boost/describe.hpp>
#include <boost/uuid/uuid.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>

namespace demo {

using namespace zoo::spider;

BOOST_DEFINE_ENUM_CLASS(Status, available, sold, cancelled)

struct Customer final
{
	std::uint64_t       id;
	std::string         name;
	std::vector<Status> statuses{};

	static Customer example();
};

BOOST_DESCRIBE_STRUCT(Customer, (), (id, name, statuses))

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

	static Error create(const std::exception& e);
	static Error create(int error_code, std::string message);

	static Error example();

	http::status status() const noexcept;

private:
	http::status status_;
};

BOOST_DESCRIBE_STRUCT(Error, (), (message, error_code, location))
BOOST_DESCRIBE_STRUCT(Error::SourceLocation, (), (file, function, line))

template<typename T>
struct Test
{
	bool succeeded;
	T    data;

	static Test<T>          example();
	static std::string_view type_name();
};

#define DESCRIBE_TEST_SPECIALIZATION(TYPE, NAME)                                                                                           \
	using NAME = Test<TYPE>;                                                                                                               \
	BOOST_DESCRIBE_STRUCT(NAME, (), (succeeded, data))

DESCRIBE_TEST_SPECIALIZATION(std::string, TestString)
DESCRIBE_TEST_SPECIALIZATION(boost::uuids::uuid, TestUuid)

template<http::status Status, typename T>
auto make_status_result(T&& result)
{
	return status_result<Status, T>{ std::move(result) };
}

} // namespace demo

// Define annotations for class members
// These have to be defined at global namespace (for now?)
SPIDER_OAS_ANNOTATE_MEMBER(demo::Error::message, "The error message")

SPIDER_OAS_ANNOTATE_MEMBER(demo::Customer::id, R"(
	The customer ID.
	This is the primary key.)"_rlws)
SPIDER_OAS_ANNOTATE_MEMBER(demo::Customer::name, "The customer name")
