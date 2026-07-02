//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/rest/concepts.hpp"
#include "zoo/spider/rest/status_result.hpp"

namespace zoo {
namespace spider {
namespace {

struct default_constructible_payload
{
	int value{};
};

// A perfectly valid payload that simply has no default constructor. status_result only ever
// constructs its value from an rvalue, so this is a legitimate result type.
struct non_default_constructible_payload
{
	int value;

	explicit non_default_constructible_payload(int v)
	    : value{ v }
	{
	}
};

} // namespace

TEST(RestConcepts, IsStatusResultRecognisesResultTypes)
{
	// Sanity: a status_result over a default-constructible payload is recognised.
	EXPECT_TRUE((IsStatusResult<status_result<status::ok, default_constructible_payload>>));

	// Regression: a status_result whose value_type has no default constructor must still be
	// recognised. The buggy concept value-initialised value_type() and so rejected this type,
	// causing the controller/OpenAPI to ignore the declared HTTP status and treat the wrapper
	// struct itself as the JSON payload.
	EXPECT_TRUE((IsStatusResult<status_result<status::created, non_default_constructible_payload>>));

	// A plain payload is not a status_result.
	EXPECT_FALSE(IsStatusResult<default_constructible_payload>);
}

} // namespace spider
} // namespace zoo
