//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/tag_invoke/greg_date.h"
#include "zoo/common/conversion/conversion.h"

namespace boost::gregorian {

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& out, const date& in)
{
	out = zoo::conversion::boost_date_to_string(in);
}

date tag_invoke(const boost::json::value_to_tag<date>&, const boost::json::value& in)
{
	return zoo::conversion::string_to_boost_date(in.as_string());
}

} // namespace boost::gregorian
