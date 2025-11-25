//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/rest/basicauthorization.h"
#include "zoo/common/misc/quoted_c.h"

#include <boost/beast/core/detail/base64.hpp>
#include <fmt/format.h>

namespace zoo {
namespace spider {
namespace {

std::optional<std::string> decode_base64(std::string_view in)
{
	using namespace boost::beast::detail;

	const auto  dest_size = base64::decoded_size(in.length());
	std::string dest{};
	dest.resize(dest_size);
	const auto [bytes_written, bytes_read] = base64::decode(dest.data(), in.data(), in.length());
	if (bytes_read < in.length())
	{
		if (in.at(bytes_read) != '=')
		{
			return std::nullopt;
		}
	}
	if (bytes_written != dest_size)
	{
		dest.resize(bytes_written);
	}
	return dest;
}

} // namespace

basic_authorization::basic_authorization(std::string_view                scheme_name,
                                         verification_callback           callback,
                                         std::string                     challenge_realm,
                                         std::optional<std::string_view> insert_username_header)
    : isecurityscheme{}
    , scheme_name_{ scheme_name }
    , callback_{ std::move(callback) }
    , challenge_realm_{ std::move(challenge_realm) }
    , inject_username_header_{ std::move(insert_username_header) }
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

std::expected<void, std::string> basic_authorization::verify(request& req, const url_view&, const std::vector<std::string_view>&) const
{
	constexpr auto header = "Authorization";
	const auto     it     = req.find(header);
	if (it == req.end())
	{
		return std::unexpected(fmt::format("{}: Missing '{}' header.", scheme_name_, header));
	}
	const auto authorization = req[header];
	if (!authorization.starts_with("Basic "))
	{
		return std::unexpected(fmt::format("{}: Invalid scheme in '{}' header.", scheme_name_, header));
	}
	const auto base64  = authorization.substr(6u);
	const auto decoded = decode_base64(base64);
	if (!decoded)
	{
		return std::unexpected(fmt::format("{}: Invalid base64 data in '{}' header.", scheme_name_, header));
	}
	const auto decoded_view = std::string_view{ decoded.value() };
	const auto colon_pos    = decoded_view.find(':');
	if (colon_pos == decoded_view.npos)
	{
		return std::unexpected(fmt::format("{}: No colon in decoded base64 data from '{}' header.", scheme_name_, header));
	}
	const auto user = decoded_view.substr(0u, colon_pos);
	const auto pass = decoded_view.substr(colon_pos + 1);
	if (!callback_(user, pass))
	{
		return std::unexpected(fmt::format("{}: Invalid user name or password.", scheme_name_));
	}
	if (inject_username_header_)
	{
		req.set(inject_username_header_.value(), user);
	}
	return {};
}

std::optional<std::string> basic_authorization::challenge() const
{
	return fmt::format("Basic realm={}", quoted_c(challenge_realm_));
}

} // namespace spider
} // namespace zoo
