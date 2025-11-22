//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/message.h"

#include <optional>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API error_response_factory final
{
public:
	static response create(const request& req, http::status status, const std::optional<string_view>& html = std::nullopt);
	static response create(http::status status, const std::optional<string_view>& html = std::nullopt);
};

template<http::status status>
class error_response final
{
public:
	static response create(const request& req)
	{
		return error_response_factory::create(req, status);
	}

	static response create()
	{
		return error_response_factory::create(status);
	}
};

using bad_request           = error_response<status::bad_request>;
using not_found             = error_response<status::not_found>;
using internal_server_error = error_response<status::internal_server_error>;
using not_implemented       = error_response<status::not_implemented>;
using method_not_allowed    = error_response<status::method_not_allowed>;

} // namespace spider
} // namespace zoo
