//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/core/types.h"

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include <string_view>
#include <optional>
#include <cstdint>

namespace zoo {
namespace squid {
namespace postgresql {

class field final
{
	std::string_view                name_;
	std::optional<std::string_view> value_;

public:
	explicit field(std::string_view name);
	explicit field(std::string_view name, std::string_view value);

	bool     is_null() const noexcept;
	explicit operator bool() const noexcept; // returns !is_null()

	std::string_view                       name() const noexcept;
	const std::optional<std::string_view>& value() const noexcept;

	// These methods throw if the value is null.
	std::string                      to_string() const;
	std::string_view                 to_string_view() const noexcept;
	byte_string                      to_byte_string() const;
	bool                             to_bool() const;
	int                              to_int() const;
	std::int8_t                      to_int8() const;
	std::int16_t                     to_int16() const;
	std::int32_t                     to_int32() const;
	std::int64_t                     to_int64() const;
	unsigned                         to_unsigned() const;
	std::uint8_t                     to_uint8() const;
	std::uint16_t                    to_uint16() const;
	std::uint32_t                    to_uint32() const;
	std::uint64_t                    to_uint64() const;
	float                            to_float() const;
	double                           to_double() const;
	long double                      to_double128() const;
	boost::posix_time::ptime         to_posix_ptime() const;
	boost::posix_time::time_duration to_posix_time_duration() const;
	boost::gregorian::date           to_gregorian_date() const;
	time_point                       to_time_point() const;
	date                             to_date() const;
	time_of_day                      to_time_of_day() const;

	// These methods return std::nullopt if the value is null.
	std::optional<std::string>                      to_optional_string() const;
	std::optional<std::string_view>                 to_optional_string_view() const noexcept;
	std::optional<byte_string>                      to_optional_byte_string() const;
	std::optional<bool>                             to_optional_bool() const;
	std::optional<int>                              to_optional_int() const;
	std::optional<std::int8_t>                      to_optional_int8() const;
	std::optional<std::int16_t>                     to_optional_int16() const;
	std::optional<std::int32_t>                     to_optional_int32() const;
	std::optional<std::int64_t>                     to_optional_int64() const;
	std::optional<unsigned>                         to_optional_unsigned() const;
	std::optional<std::uint8_t>                     to_optional_uint8() const;
	std::optional<std::uint16_t>                    to_optional_uint16() const;
	std::optional<std::uint32_t>                    to_optional_uint32() const;
	std::optional<std::uint64_t>                    to_optional_uint64() const;
	std::optional<float>                            to_optional_float() const;
	std::optional<double>                           to_optional_double() const;
	std::optional<long double>                      to_optional_double128() const;
	std::optional<boost::posix_time::ptime>         to_optional_posix_ptime() const;
	std::optional<boost::posix_time::time_duration> to_optional_posix_time_duration() const;
	std::optional<boost::gregorian::date>           to_optional_gregorian_date() const;
	std::optional<time_point>                       to_optional_time_point() const;
	std::optional<date>                             to_optional_date() const;
	std::optional<time_of_day>                      to_optional_time_of_day() const;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
