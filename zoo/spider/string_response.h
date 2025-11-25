//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/message.h"

#include <string_view>
#include <string>
#include <cstdint>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API string_response final
{
public:
	static response create(const request& req, http::status status, std::string_view content_type, std::string_view body);
	static response create(http::status status, std::string_view content_type, std::string_view body);

	static response create(const request& req, http::status status, std::string_view content_type, std::string body);
	static response create(http::status status, std::string_view content_type, std::string body);
};

} // namespace spider
} // namespace zoo
