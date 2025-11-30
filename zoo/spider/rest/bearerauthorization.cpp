//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/rest/bearerauthorization.h"
#include "zoo/common/misc/quoted_c.h"

#include <fmt/format.h>

namespace zoo {
namespace spider {
namespace {

} // namespace

bearer_authorization::bearer_authorization(std::string_view scheme_name, verification_callback callback, std::string challenge_realm)
    : isecurityscheme{}
    , scheme_name_{ scheme_name }
    , callback_{ std::move(callback) }
    , challenge_realm_{ std::move(challenge_realm) }
{
}

bearer_authorization::~bearer_authorization() = default;

std::string_view bearer_authorization::scheme_name() const
{
	return scheme_name_;
}

boost::json::object bearer_authorization::scheme() const
{
	return { { "type", "http" }, { "scheme", "bearer" } };
}

std::expected<auth_data, auth_error> bearer_authorization::verify(request& req, const url_view&, const std::vector<std::string_view>&) const
{
	constexpr auto header = "Authorization";
	const auto     it     = req.find(header);
	if (it == req.end())
	{
		return std::unexpected(make_verification_error(fmt::format("{}: Missing '{}' header.", scheme_name_, header), {}));
	}
	const auto authorization = req[header];
	if (!authorization.starts_with("Bearer "))
	{
		return std::unexpected(make_verification_error(fmt::format("{}: Invalid scheme in '{}' header.", scheme_name_, header), {}));
	}
	const auto token = authorization.substr(7u);
	return callback_(*this, token);
}

auth_error bearer_authorization::make_verification_error(std::string                                           message,
                                                         std::vector<std::pair<std::string_view, std::string>> auth_params) const
{
	auth_params.insert(auth_params.begin(), std::make_pair("realm", challenge_realm_));
	return auth_error{ .message    = std::move(message),
		               .challenges = { { .auth_scheme = "Bearer", .auth_params = std::move(auth_params) } } };
}

} // namespace spider
} // namespace zoo
