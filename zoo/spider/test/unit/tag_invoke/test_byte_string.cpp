//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/tag_invoke/byte_string.h"
#include "zoo/common/misc/byte_string.h"

#include <boost/json/value.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>

namespace zoo {
namespace spider {
namespace {

TEST(ByteStringTagInvoke, ValidBase64RoundTrips)
{
	zoo::byte_string original;
	original.push_back(0x00);
	original.push_back(0x01);
	original.push_back(0xfe);
	original.push_back(0xff);

	const auto jv      = boost::json::value_from(original);
	const auto decoded = boost::json::value_to<zoo::byte_string>(jv);

	EXPECT_EQ(decoded, original);
}

// Regression: an invalid-base64 JSON string must be rejected, not silently converted to an empty
// byte_string (which hid malformed input from the caller).
TEST(ByteStringTagInvoke, InvalidBase64Throws)
{
	const boost::json::value jv = "!!!! not base64 !!!!";
	EXPECT_ANY_THROW((void)boost::json::value_to<zoo::byte_string>(jv));
}

} // namespace
} // namespace spider
} // namespace zoo
