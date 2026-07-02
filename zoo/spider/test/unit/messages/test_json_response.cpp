//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/messages/json_response.h"
#include "zoo/spider/messages/message.h"

#include <boost/beast/http/verb.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_from.hpp>

namespace zoo {
namespace spider {
namespace {

// A type whose value_from records whether it was serialized from an rvalue (moved) or an lvalue.
struct move_probe
{
	bool* moved_from;
};

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& jv, move_probe&& p)
{
	if (p.moved_from)
	{
		*p.moved_from = true;
	}
	jv = "probe";
}

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& jv, const move_probe&)
{
	jv = "probe";
}

request make_get()
{
	return request{ http::verb::get, "/", 11 };
}

} // namespace

// Passing a named lvalue to json_response::create must not move from it (the greedy T&& overload
// used to bind lvalues and value_from(std::move(data)) silently emptied the caller's object).
TEST(JsonResponse, DoesNotMoveFromNamedLvalue)
{
	bool       moved = false;
	move_probe p{ &moved };
	const auto req = make_get();
	(void)json_response::create(req, http::status::ok, p);
	EXPECT_FALSE(moved);
}

// An rvalue may still be moved.
TEST(JsonResponse, MovesFromRvalue)
{
	bool       moved = false;
	const auto req   = make_get();
	(void)json_response::create(req, http::status::ok, move_probe{ &moved });
	EXPECT_TRUE(moved);
}

} // namespace spider
} // namespace zoo
