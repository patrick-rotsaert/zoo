//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/rest/apikeyauthorization.h"

#include <fmt/format.h>

namespace zoo {
namespace spider {
namespace {

using source = api_key_authorization::source;

std::string_view source_name(source in)
{
	switch (in)
	{
	case api_key_authorization::source::header:
		return "header";
	case api_key_authorization::source::query:
		return "query";
	}
	std::unreachable();
}

auth_error make_verification_error(std::string message)
{
	return auth_error{ .message = std::move(message), .challenges = {} };
}

} // namespace

api_key_authorization::api_key_authorization(std::string_view scheme_name, source in, std::string_view name, std::string key)
    : isecurityscheme{}
    , scheme_name_{ scheme_name }
    , in_{ in }
    , name_{ name }
    , key_{ std::move(key) }
{
}

api_key_authorization::~api_key_authorization() = default;

std::string_view api_key_authorization::scheme_name() const
{
	return scheme_name_;
}

boost::json::object api_key_authorization::scheme() const
{
	return { { "type", "apiKey" }, { "in", source_name(in_) }, { "name", name_ } };
}

std::expected<auth_data, auth_error>
api_key_authorization::verify(request& req, const url_view& url, const std::vector<std::string_view>&) const
{
	std::string_view key;
	switch (in_)
	{
	case source::header:
	{
		const auto it = req.find(name_);
		if (it == req.end())
		{
			return std::unexpected(make_verification_error(fmt::format("{}: Missing '{}' header.", scheme_name_, name_)));
		}
		else
		{
			key = req[name_];
		}
	}
	break;
	case source::query:
	{
		const auto it = url.params().find(name_);
		if (it == url.params().end())
		{
			return std::unexpected(make_verification_error(fmt::format("{}: Missing '{}' query parameter.", scheme_name_, name_)));
		}
		else
		{
			key = (*it).value;
		}
	}
	break;
	}

	if (key == key_)
	{
		return auth_data{};
	}
	else
	{
		return std::unexpected(make_verification_error(fmt::format("{}: Bad API key.", scheme_name_)));
	}
}

} // namespace spider
} // namespace zoo
