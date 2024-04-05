//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/tag_invoke/date.h"
#include "zoo/common/conversion/conversion.h"

namespace std::chrono {

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& out, const year_month_day& in)
{
	out = zoo::conversion::date_to_string(in);
}

year_month_day tag_invoke(const boost::json::value_to_tag<year_month_day>&, const boost::json::value& in)
{
	return zoo::conversion::string_to_date(in.as_string());
}

} // namespace std::chrono
