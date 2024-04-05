//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "zoo/common/conversion/conversion.h"
#include "zoo/common/misc/throw_exception.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <fmt/format.h>

#include <optional>
#include <regex>
#include <cmath>
#include <cassert>

#if defined(_MSC_VER)
#pragma warning(disable : 4456)
#endif

namespace zoo {
namespace conversion {

namespace {

struct parsed_time_point final
{
	int                   year;
	unsigned              month;
	unsigned              day;
	unsigned              hours;
	unsigned              minutes;
	unsigned              seconds;
	std::optional<double> fracseconds;
	std::optional<int>    utc_offset_minutes;
};

parsed_time_point parse_time_point(std::string_view in)
{
	parsed_time_point out{};

	static const std::regex re{ R"(^(\d{4})-(\d{2})-(\d{2})[T ](\d{2}):(\d{2}):(\d{2})(\.\d*)?( ?([+-]\d{2})(:?(\d{2}))?|Z)?$)" };
	//                              1       2       3          4       5       6      7       8  9          10 11

	std::match_results<std::string_view::const_iterator> matches;
	if (!std::regex_match(in.begin(), in.end(), matches, re) || matches.size() != 1 + 11)
	{
		ZOO_THROW_EXCEPTION(std::invalid_argument{ "invalid time point format" });
	}

	out.year    = string_to_number<int>(matches[1].str());
	out.month   = string_to_number<unsigned>(matches[2].str());
	out.day     = string_to_number<unsigned>(matches[3].str());
	out.hours   = string_to_number<unsigned>(matches[4].str());
	out.minutes = string_to_number<unsigned>(matches[5].str());
	out.seconds = string_to_number<unsigned>(matches[6].str());

	if (matches.length(7) > 1)
	{
		out.fracseconds = string_to_number<double>(matches[7].str());
	}

	if (matches.length(9))
	{
		const auto hours            = matches[9].str();
		const auto utc_offset_hours = string_to_number<int>((hours.front() == '+') ? &hours[1] : hours);
		out.utc_offset_minutes      = utc_offset_hours * 60;

		if (matches.length(11))
		{
			out.utc_offset_minutes.value() += string_to_number<int>(matches[11].str()) * (utc_offset_hours < 0 ? -1 : 1);
		}
	}

	return out;
}

struct parsed_date final
{
	int      year;
	unsigned month;
	unsigned day;
};

parsed_date parse_date(std::string_view in)
{
	auto out = parsed_date{};

	static const std::regex re1{ R"(^(-?\d{1,4})-(\d{2})-(\d{2})$)" };
	//                              1            2        3
	static const std::regex re2{ R"(^(-?\d{4})(\d{2})(\d{2})$)" };
	//                               1        2      3

	auto matches = std::match_results<std::string_view::const_iterator>{};
	if (!std::regex_match(in.begin(), in.end(), matches, re1) || matches.size() != 1 + 3)
	{
		if (!std::regex_match(in.begin(), in.end(), matches, re2) || matches.size() != 1 + 3)
		{
			ZOO_THROW_EXCEPTION(std::invalid_argument{ "invalid date format" });
		}
	}

	out.year  = string_to_number<int>(matches[1].str());
	out.month = string_to_number<unsigned>(matches[2].str());
	out.day   = string_to_number<unsigned>(matches[3].str());

	return out;
}

struct parsed_time_of_day final
{
	unsigned              hours;
	unsigned              minutes;
	unsigned              seconds;
	std::optional<double> fracseconds;
	std::optional<int>    utc_offset_minutes;
};

parsed_time_of_day parse_time_of_day(std::string_view in)
{
	auto out = parsed_time_of_day{};

	static const std::regex re{ R"(^(\d{2}):(\d{2}):(\d{2})(\.\d*)?( ?([+-]\d{2})(:?(\d{2}))?)?$)" };
	//                              1       2       3      4       5  6          7  8

	std::match_results<std::string_view::const_iterator> matches;
	if (!std::regex_match(in.begin(), in.end(), matches, re) || matches.size() != 1 + 8)
	{
		ZOO_THROW_EXCEPTION(std::invalid_argument{ "invalid time of day format" });
	}

	out.hours   = string_to_number<unsigned>(matches[1].str());
	out.minutes = string_to_number<unsigned>(matches[2].str());
	out.seconds = string_to_number<unsigned>(matches[3].str());

	if (matches.length(4) > 1)
	{
		out.fracseconds = string_to_number<double>(matches[4].str());
	}

	if (matches.length(6))
	{
		const auto hours            = matches[6].str();
		const auto utc_offset_hours = string_to_number<int>((hours.front() == '+') ? &hours[1] : hours);
		out.utc_offset_minutes      = utc_offset_hours * 60;

		if (matches.length(8))
		{
			out.utc_offset_minutes.value() += string_to_number<int>(matches[8].str()) * (utc_offset_hours < 0 ? -1 : 1);
		}
	}

	return out;
}

} // namespace

void string_to_time_point(std::string_view in, time_point& out)
{
	// TODO: Use std::chrono::parse when it becomes available in libstdc++

	// https://github.com/HowardHinnant/date.git would be a great alternative,
	// but I prefer not to have any additional dependencies other than the database client libraries.

	const auto parsed = parse_time_point(in);

	out = std::chrono::sys_days{ std::chrono::year{ parsed.year } / std::chrono::month{ parsed.month } / parsed.day } +
	      std::chrono::hours{ parsed.hours } + std::chrono::minutes{ parsed.minutes } + std::chrono::seconds{ parsed.seconds };

	if (parsed.fracseconds)
	{
		out += std::chrono::microseconds{ static_cast<uint32_t>(parsed.fracseconds.value() * 1e6 + .5) };
	}

	if (parsed.utc_offset_minutes)
	{
		out -= std::chrono::minutes{ parsed.utc_offset_minutes.value() };
	}
}

time_point string_to_time_point(std::string_view in)
{
	auto result = time_point{};
	string_to_time_point(in, result);
	return result;
}

void string_to_date(std::string_view in, date& out)
{
	// TODO: Use std::chrono::parse when it becomes available in libstdc++

	// https://github.com/HowardHinnant/date.git would be a great alternative,
	// but I prefer not to have any additional dependencies other than the database client libraries.

	const auto parsed = parse_date(in);

	out = std::chrono::sys_days{ std::chrono::year{ parsed.year } / std::chrono::month{ parsed.month } / parsed.day };
}

date string_to_date(std::string_view in)
{
	auto result = date{};
	string_to_date(in, result);
	return result;
}

void string_to_time_of_day(std::string_view in, time_of_day& out)
{
	// TODO: Use std::chrono::parse when it becomes available in libstdc++

	// https://github.com/HowardHinnant/date.git would be a great alternative,
	// but I prefer not to have any additional dependencies other than the database client libraries.

	const auto parsed = parse_time_of_day(in);

	auto tmp = std::chrono::microseconds{ (3600LL * parsed.hours + 60 * parsed.minutes + parsed.seconds) * 1000000LL };

	if (parsed.fracseconds)
	{
		tmp += std::chrono::microseconds{ static_cast<uint32_t>(parsed.fracseconds.value() * 1e6 + .5) };
	}

	if (parsed.utc_offset_minutes)
	{
		tmp -= std::chrono::minutes{ parsed.utc_offset_minutes.value() };
	}

	out = time_of_day{ tmp };
}

time_of_day string_to_time_of_day(std::string_view in)
{
	auto result = time_of_day{};
	string_to_time_of_day(in, result);
	return result;
}

void time_point_to_string(const time_point& in, std::string& out, const char date_time_separator)
{
	// TODO: use std::format when it becomes available in libstdc++
	using namespace std::chrono;
	auto dp   = floor<days>(in);
	auto date = year_month_day{ dp };
	auto time = hh_mm_ss{ floor<microseconds>(in - dp) };

	if (time.subseconds().count())
	{
		out = fmt::format("{:04d}-{:02d}-{:02d}{}{:02d}:{:02d}:{:02d}.{:06d}",
		                  static_cast<int>(date.year()),
		                  static_cast<unsigned>(date.month()),
		                  static_cast<unsigned>(date.day()),
		                  date_time_separator,
		                  static_cast<long>(time.hours().count()),
		                  static_cast<long>(time.minutes().count()),
		                  static_cast<long>(time.seconds().count()),
		                  static_cast<long>(time.subseconds().count()));
	}
	else
	{
		out = fmt::format("{:04d}-{:02d}-{:02d}{}{:02d}:{:02d}:{:02d}",
		                  static_cast<int>(date.year()),
		                  static_cast<unsigned>(date.month()),
		                  static_cast<unsigned>(date.day()),
		                  date_time_separator,
		                  static_cast<long>(time.hours().count()),
		                  static_cast<long>(time.minutes().count()),
		                  static_cast<long>(time.seconds().count()));
	}
}

std::string time_point_to_string(const time_point& in, const char date_time_separator)
{
	auto result = std::string{};
	time_point_to_string(in, result, date_time_separator);
	return result;
}

void time_point_to_iso8601(const time_point& in, std::string& out)
{
	return time_point_to_string(in, out, 'T');
}

std::string time_point_to_iso8601(const time_point& in)
{
	return time_point_to_string(in, 'T');
}

void time_point_to_sql(const time_point& in, std::string& out)
{
	return time_point_to_string(in, out, ' ');
}

std::string time_point_to_sql(const time_point& in)
{
	return time_point_to_string(in, ' ');
}

void date_to_string(const date& in, std::string& out)
{
	out = fmt::format(
	    "{:04d}-{:02d}-{:02d}", static_cast<int>(in.year()), static_cast<unsigned>(in.month()), static_cast<unsigned>(in.day()));
}

std::string date_to_string(const date& in)
{
	auto result = std::string{};
	date_to_string(in, result);
	return result;
}

void time_of_day_to_string(const time_of_day& in, std::string& out)
{
	// TODO: use std::format when it becomes available in libstdc++
	if (in.subseconds().count())
	{
		out = fmt::format("{:02d}:{:02d}:{:02d}.{:06d}",
		                  static_cast<long>(in.hours().count()),
		                  static_cast<long>(in.minutes().count()),
		                  static_cast<long>(in.seconds().count()),
		                  static_cast<long>(in.subseconds().count()));
	}
	else
	{
		out = fmt::format("{:02d}:{:02d}:{:02d}",
		                  static_cast<long>(in.hours().count()),
		                  static_cast<long>(in.minutes().count()),
		                  static_cast<long>(in.seconds().count()));
	}
}

std::string time_of_day_to_string(const time_of_day& in)
{
	auto result = std::string{};
	time_of_day_to_string(in, result);
	return result;
}

void boost_ptime_to_string(const boost::posix_time::ptime& in, std::string& out, const char date_time_separator)
{
	out = boost_ptime_to_string(in, date_time_separator);
}

std::string boost_ptime_to_string(const boost::posix_time::ptime& in, const char date_time_separator)
{
	auto tmp = boost::posix_time::to_iso_extended_string(in);
	// Expected format of tmp is YYYY-MM-DDTHH:MM:SS[.fffffffff]
	// Format to return is YYYY-MM-DD HH:MM:SS.fffffffff
	// So just replace the 'T' with a space.
	if (date_time_separator != 'T' && tmp.length() >= 19u && tmp[10u] == 'T')
	{
		tmp[10u] = date_time_separator;
	}
	return tmp;
}

void boost_ptime_to_iso8601(const boost::posix_time::ptime& in, std::string& out)
{
	return boost_ptime_to_string(in, out, 'T');
}

std::string boost_ptime_to_iso8601(const boost::posix_time::ptime& in)
{
	return boost_ptime_to_string(in, 'T');
}

void boost_ptime_to_sql(const boost::posix_time::ptime& in, std::string& out)
{
	return boost_ptime_to_string(in, out, ' ');
}

std::string boost_ptime_to_sql(const boost::posix_time::ptime& in)
{
	return boost_ptime_to_string(in, ' ');
}

void boost_date_to_string(const boost::gregorian::date& in, std::string& out)
{
	out = boost::gregorian::to_iso_extended_string(in);
}

std::string boost_date_to_string(const boost::gregorian::date& in)
{
	return boost::gregorian::to_iso_extended_string(in);
}

void boost_time_duration_to_string(const boost::posix_time::time_duration& in, std::string& out)
{
	out = boost::posix_time::to_simple_string(in);
}

std::string boost_time_duration_to_string(const boost::posix_time::time_duration& in)
{
	return boost::posix_time::to_simple_string(in);
}

void string_to_boost_ptime(std::string_view in, boost::posix_time::ptime& out)
{
	const auto parsed = parse_time_point(in);

	out = boost::posix_time::ptime{ boost::gregorian::date{ static_cast<short unsigned int>(parsed.year),
		                                                    static_cast<short unsigned int>(parsed.month),
		                                                    static_cast<short unsigned int>(parsed.day) },
		                            boost::posix_time::time_duration{ parsed.hours, parsed.minutes, parsed.seconds } };

	if (parsed.fracseconds)
	{
		out += boost::posix_time::microseconds{ static_cast<uint32_t>(parsed.fracseconds.value() * 1e6 + .5) };
	}

	if (parsed.utc_offset_minutes)
	{
		out -= boost::posix_time::minutes{ parsed.utc_offset_minutes.value() };
	}
}

boost::posix_time::ptime string_to_boost_ptime(std::string_view in)
{
	auto out = boost::posix_time::ptime{};
	string_to_boost_ptime(in, out);
	return out;
}

void string_to_boost_date(std::string_view in, boost::gregorian::date& out)
{
	const auto parsed = parse_date(in);

	if (parsed.year < 0)
	{
		ZOO_THROW_EXCEPTION(std::invalid_argument{ "boost date does not support negative years" });
	}

	out = boost::gregorian::date{ static_cast<short unsigned int>(parsed.year),
		                          static_cast<short unsigned int>(parsed.month),
		                          static_cast<short unsigned int>(parsed.day) };
}

boost::gregorian::date string_to_boost_date(std::string_view in)
{
	auto out = boost::gregorian::date{};
	string_to_boost_date(in, out);
	return out;
}

void string_to_boost_time_duration(std::string_view in, boost::posix_time::time_duration& out)
{
	const auto parsed = parse_time_of_day(in);

	out = boost::posix_time::time_duration{ parsed.hours, parsed.minutes, parsed.seconds };

	if (parsed.fracseconds)
	{
		out += boost::posix_time::microseconds{ static_cast<uint32_t>(parsed.fracseconds.value() * 1e6 + .5) };
	}

	if (parsed.utc_offset_minutes)
	{
		out -= boost::posix_time::minutes{ parsed.utc_offset_minutes.value() };
	}
}

boost::posix_time::time_duration string_to_boost_time_duration(std::string_view in)
{
	auto out = boost::posix_time::time_duration{};
	string_to_boost_time_duration(in, out);
	return out;
}

} // namespace conversion
} // namespace zoo
