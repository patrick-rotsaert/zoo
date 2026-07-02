//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/rest/basicauthorization.h"
#include "zoo/spider/rest/bearerauthorization.h"
#include "zoo/spider/rest/base64.h"

#include <boost/beast/http/verb.hpp>
#include <boost/url/parse.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace zoo {
namespace spider {
namespace {

request make_request(const std::string& authorization)
{
	request req{ http::verb::get, "/", 11 };
	req.set("Authorization", authorization);
	return req;
}

} // namespace

// RFC 7235: the auth-scheme token is case-insensitive, so a conformant client sending "basic"
// (or "BASIC") must be accepted.
TEST(AuthScheme, BasicSchemeNameIsCaseInsensitive)
{
	basic_authorization auth{ "BasicAuth",
		                      [](std::string_view user, std::string_view pass) -> std::expected<auth_data, std::string> {
		                          if (user == "me" && pass == "secret")
		                          {
			                          return auth_data{};
		                          }
		                          return std::unexpected(std::string{ "bad credentials" });
		                      },
		                      "realm" };

	const auto url = boost::urls::parse_uri_reference("/").value();
	auto       req = make_request("basic " + base64::encode("me:secret"));

	EXPECT_TRUE(auth.verify(req, url, {}).has_value());
}

TEST(AuthScheme, BearerSchemeNameIsCaseInsensitive)
{
	bearer_authorization auth{ "BearerAuth",
		                       [](const bearer_authorization&, std::string_view token) -> std::expected<auth_data, auth_error> {
		                           if (token == "the-token")
		                           {
			                           return auth_data{};
		                           }
		                           return std::unexpected(auth_error{ .message = "bad token", .challenges = {} });
		                       },
		                       "realm" };

	const auto url = boost::urls::parse_uri_reference("/").value();
	auto       req = make_request("bearer the-token");

	EXPECT_TRUE(auth.verify(req, url, {}).has_value());
}

// The scheme name may be supplied as a temporary; the object must own it (regression guard for the
// non-owning string_view member design).
TEST(AuthScheme, SchemeNameIsOwnedFromTemporary)
{
	basic_authorization auth{ std::string{ "TemporaryScheme" },
		                      [](std::string_view, std::string_view) -> std::expected<auth_data, std::string> { return auth_data{}; },
		                      "realm" };

	EXPECT_EQ(auth.scheme_name(), "TemporaryScheme");
}

} // namespace spider
} // namespace zoo
