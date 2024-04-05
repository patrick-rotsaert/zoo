//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/tag_invoke/time_duration.h"
#include "zoo/common/conversion/conversion.h"

namespace boost::posix_time {

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& out, const time_duration& in)
{
	out = zoo::conversion::boost_time_duration_to_string(in);
}

time_duration tag_invoke(const boost::json::value_to_tag<time_duration>&, const boost::json::value& in)
{
	return zoo::conversion::string_to_boost_time_duration(in.as_string());
}

} // namespace boost::posix_time
