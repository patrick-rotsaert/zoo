//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/core/config.h"
#include "zoo/squid/core/types.h"
#include "zoo/common/misc/is_optional.hpp"

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

#include <variant>
#include <cstdint>
#include <string>
#include <optional>
#include <type_traits>

namespace zoo {
namespace squid {

/// This class holds a pointer to a bound result column.
class ZOO_SQUID_CORE_API result final
{
public:
	using non_nullable_type = std::variant< //
	    bool*,
	    char*,
	    signed char*,
	    unsigned char*,
	    std::int16_t*,
	    std::uint16_t*,
	    std::int32_t*,
	    std::uint32_t*,
	    std::int64_t*,
	    std::uint64_t*,
	    float*,
	    double*,
	    long double*,
	    std::string*,
	    byte_string*,
	    boost::posix_time::ptime*,
	    boost::gregorian::date*,
	    boost::posix_time::time_duration*,
	    time_point*,
	    date*,
	    time_of_day*
	    //
	    >;

	using nullable_type = std::variant< //
	    std::optional<bool>*,
	    std::optional<char>*,
	    std::optional<signed char>*,
	    std::optional<unsigned char>*,
	    std::optional<std::int16_t>*,
	    std::optional<std::uint16_t>*,
	    std::optional<std::int32_t>*,
	    std::optional<std::uint32_t>*,
	    std::optional<std::int64_t>*,
	    std::optional<std::uint64_t>*,
	    std::optional<float>*,
	    std::optional<double>*,
	    std::optional<long double>*,
	    std::optional<std::string>*,
	    std::optional<byte_string>*,
	    std::optional<boost::posix_time::ptime>*,
	    std::optional<boost::gregorian::date>*,
	    std::optional<boost::posix_time::time_duration>*,
	    std::optional<time_point>*,
	    std::optional<date>*,
	    std::optional<time_of_day>*
	    //
	    >;

	using type = std::variant<non_nullable_type, nullable_type>;

	template<typename T>
	explicit result(T& value)
	    : value_{ get_value(value) }
	{
	}

	result(const result&)            = default;
	result(result&& src)             = default;
	result& operator=(const result&) = default;
	result& operator=(result&&)      = default;

	/// Get the value
	const type& value() const noexcept;

private:
	template<typename T>
	static type get_value(T& value)
	{
		if constexpr (is_optional_v<T>)
		{
			using V = typename T::value_type;
			if constexpr (std::is_enum_v<V>)
			{
				return nullable_type{ reinterpret_cast<std::optional<std::underlying_type_t<V>>*>(&value) };
			}
			else
			{
				return nullable_type{ &value };
			}
		}
		else if constexpr (std::is_enum_v<T>)
		{
			return non_nullable_type{ reinterpret_cast<std::underlying_type_t<T>*>(&value) };
		}
		else
		{
			return non_nullable_type{ &value };
		}
	}

private:
	type value_;
};

} // namespace squid
} // namespace zoo
