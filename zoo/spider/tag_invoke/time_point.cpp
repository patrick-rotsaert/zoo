//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/tag_invoke/time_point.h"
#include "zoo/common/conversion/conversion.h"

namespace std::chrono {

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& out, const system_clock::time_point& in)
{
	out = zoo::conversion::time_point_to_iso8601(in);
}

system_clock::time_point tag_invoke(const boost::json::value_to_tag<system_clock::time_point>&, const boost::json::value& in)
{
	return zoo::conversion::string_to_time_point(in.as_string());
}

} // namespace std::chrono
