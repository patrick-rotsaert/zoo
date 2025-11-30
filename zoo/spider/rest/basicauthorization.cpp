//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/rest/basicauthorization.h"
#include "zoo/spider/rest/base64.h"
#include "zoo/common/misc/quoted_c.h"

#include <fmt/format.h>

namespace zoo {
namespace spider {

basic_authorization::basic_authorization(std::string_view scheme_name, verification_callback callback, std::string challenge_realm)
    : isecurityscheme{}
    , scheme_name_{ scheme_name }
    , callback_{ std::move(callback) }
    , challenge_realm_{ std::move(challenge_realm) }
{
}

basic_authorization::~basic_authorization() = default;

std::string_view basic_authorization::scheme_name() const
{
	return scheme_name_;
}

boost::json::object basic_authorization::scheme() const
{
	return { { "type", "http" }, { "scheme", "basic" } };
}

std::expected<auth_data, auth_error> basic_authorization::verify(request& req, const url_view&, const std::vector<std::string_view>&) const
{
	constexpr auto header = "Authorization";
	const auto     it     = req.find(header);
	if (it == req.end())
	{
		return std::unexpected(make_verification_error(fmt::format("{}: Missing '{}' header.", scheme_name_, header)));
	}
	const auto authorization = req[header];
	if (!authorization.starts_with("Basic "))
	{
		return std::unexpected(make_verification_error(fmt::format("{}: Invalid scheme in '{}' header.", scheme_name_, header)));
	}
	const auto token   = authorization.substr(6u);
	const auto decoded = base64::decode_to_string(token);
	if (!decoded)
	{
		return std::unexpected(make_verification_error(fmt::format("{}: Invalid base64 data in '{}' header.", scheme_name_, header)));
	}
	const auto decoded_view = std::string_view{ decoded.value() };
	const auto colon_pos    = decoded_view.find(':');
	if (colon_pos == decoded_view.npos)
	{
		return std::unexpected(
		    make_verification_error(fmt::format("{}: No colon in decoded base64 data from '{}' header.", scheme_name_, header)));
	}
	const auto user = decoded_view.substr(0u, colon_pos);
	const auto pass = decoded_view.substr(colon_pos + 1);
	if (auto verified = callback_(user, pass); verified)
	{
		return std::move(verified.value());
	}
	else
	{
		return std::unexpected(make_verification_error(fmt::format("{}: {}", scheme_name_, verified.error())));
	}
}

auth_error basic_authorization::make_verification_error(std::string message) const
{
	return auth_error{ .message    = std::move(message),
		               .challenges = {
		                   { .auth_scheme = "Basic", .auth_params = { { "realm", challenge_realm_ }, { "charset", "UTF-8" } } } } };
}

} // namespace spider
} // namespace zoo
