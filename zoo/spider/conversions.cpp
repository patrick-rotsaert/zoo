//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/conversions.h"
#include "zoo/spider/aliases.h"

#include <boost/beast/core/string.hpp>

namespace zoo {
namespace spider {

std::string conversions::from_string_view(std::string_view in, const std::string* const)
{
	return std::string{ in };
}

bool conversions::from_string_view(std::string_view in, const bool* const)
{
	if (beast::iequals(in, "true") || beast::iequals(in, "yes") || beast::iequals(in, "on"))
	{
		return true;
	}
	else if (beast::iequals(in, "false") || beast::iequals(in, "no") || beast::iequals(in, "off"))
	{
		return false;
	}
	else
	{
		return conversion::string_to_number<int>(in) != 0;
	}
}

boost::gregorian::date conversions::from_string_view(std::string_view in, const boost::gregorian::date* const)
{
	return conversion::string_to_boost_date(in);
}

boost::posix_time::time_duration conversions::from_string_view(std::string_view in, const boost::posix_time::time_duration* const)
{
	return conversion::string_to_boost_time_duration(in);
}

boost::posix_time::ptime conversions::from_string_view(std::string_view in, const boost::posix_time::ptime* const)
{
	return conversion::string_to_boost_ptime(in);
}

conversions::date conversions::from_string_view(std::string_view in, const date* const)
{
	return conversion::string_to_date(in);
}

conversions::time_of_day conversions::from_string_view(std::string_view in, const time_of_day* const)
{
	return conversion::string_to_time_of_day(in);
}

conversions::time_point conversions::from_string_view(std::string_view in, const time_point* const)
{
	return conversion::string_to_time_point(in);
}

} // namespace spider
} // namespace zoo
