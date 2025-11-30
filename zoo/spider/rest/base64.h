//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"

#include <string_view>
#include <string>
#include <optional>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API base64
{
public:
	static std::optional<std::string> decode_to_string(std::string_view in);
	static std::string                encode(std::string_view in);
};

} // namespace spider
} // namespace zoo
