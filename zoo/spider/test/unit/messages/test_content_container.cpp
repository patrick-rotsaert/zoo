//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/messages/content_container.hpp"

#include <type_traits>
#include <vector>

namespace zoo {
namespace spider {

// content_container holds views into its own storage, so it needs a hand-written move that rebuilds
// those views. It must be nothrow-move-assignable (not just move-constructible) so it can be
// reassigned and stored/grown in a std::vector.
TEST(ContentContainer, IsNothrowMovable)
{
	EXPECT_TRUE(std::is_nothrow_move_constructible_v<html_container>);
	EXPECT_TRUE(std::is_nothrow_move_assignable_v<html_container>);
}

TEST(ContentContainer, MoveAssignmentRebuildsSelfViews)
{
	auto a = html_container::create("text/html", "aaaa");
	auto b = html_container::create("text/html", "bbbbbb");

	a = std::move(b);

	// The views must now point into a's own (moved-in) storage, not into the moved-from b.
	EXPECT_EQ(a.content(), "bbbbbb");
	EXPECT_EQ(a.content_type(), "text/html");

	// Also works inside a vector (which requires nothrow-move-assignable to reallocate).
	std::vector<html_container> v;
	v.push_back(html_container::create("text/html", "one"));
	v.push_back(html_container::create("text/html", "two"));
	EXPECT_EQ(v.back().content(), "two");
}

} // namespace spider
} // namespace zoo
