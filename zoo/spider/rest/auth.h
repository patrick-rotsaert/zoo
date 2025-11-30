//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <memory>

namespace zoo {
namespace spider {

struct challenge final
{
	std::string_view                                      auth_scheme;
	std::vector<std::pair<std::string_view, std::string>> auth_params;
};

struct auth_error final
{
	std::string            message;
	std::vector<challenge> challenges;
};

struct ZOO_SPIDER_API auth_data_base
{
	virtual ~auth_data_base();
};

struct ZOO_SPIDER_API www_authenticate final
{
	static std::string encode(const challenge& c);
};

using auth_data = std::unique_ptr<auth_data_base>;
using auth_map  = std::unordered_map<std::string_view, auth_data>;

} // namespace spider
} // namespace zoo
