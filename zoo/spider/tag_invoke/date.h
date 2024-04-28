//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"

#include <boost/json/conversion.hpp>

#include <chrono>

namespace std::chrono {

void ZOO_SPIDER_API           tag_invoke(const boost::json::value_from_tag&, boost::json::value& out, const year_month_day& in);
year_month_day ZOO_SPIDER_API tag_invoke(const boost::json::value_to_tag<year_month_day>&, const boost::json::value& in);

} // namespace std::chrono
