//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"

#include <boost/json/value.hpp>

#include <string>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API json_util final
{
public:
	static std::string pretty_print(const boost::json::value& in);
};

} // namespace spider
} // namespace zoo
