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
#include "zoo/common/misc/byte_string.h"

#include <boost/beast/http/buffer_body.hpp>

#include <string_view>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API buffer_response final
{
public:
	using response = http::response<http::buffer_body>;

	static response create(const request& req, http::status status, std::string_view content_type, byte_string_view body);
	static response create(http::status status, std::string_view content_type, byte_string_view body);
};

} // namespace spider
} // namespace zoo
