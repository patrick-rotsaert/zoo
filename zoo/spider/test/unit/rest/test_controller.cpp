//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/rest/controller.hpp"

#include <boost/describe/class.hpp>

#include <string>
#include <type_traits>

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

} // namespace

// A rest_controller's registered handlers capture the controller's `this`, and its router_ is a
// shared_ptr that is typically shared into a parent router. Copying/moving the controller would
// leave those handlers pointing at the original (or a moved-from) object -> dangling this. The
// controller must therefore not be copyable or movable.
TEST(RestController, IsNeitherCopyableNorMovable)
{
	using controller = rest_controller<test_error>;

	EXPECT_FALSE(std::is_copy_constructible_v<controller>);
	EXPECT_FALSE(std::is_copy_assignable_v<controller>);
	EXPECT_FALSE(std::is_move_constructible_v<controller>);
	EXPECT_FALSE(std::is_move_assignable_v<controller>);
}

} // namespace spider
} // namespace zoo
