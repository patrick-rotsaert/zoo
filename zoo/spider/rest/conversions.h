//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/common/conversion/conversion.h"
#include "zoo/common/misc/throw_exception.h"
#include "zoo/common/misc/demangled_type_name.hpp"

#include <boost/optional/optional.hpp>
#include "boost/json/conversion.hpp"
#include "boost/describe/enum_from_string.hpp"
#include <boost/uuid/uuid.hpp>

#include <fmt/format.h>

#include <string>
#include <optional>
#include <concepts>
#include <stdexcept>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API conversions final
{
public:
	using time_point  = conversion::time_point;
	using date        = conversion::date;
	using time_of_day = conversion::time_of_day;

	static std::string_view                 from_string_view(std::string_view in, const std::string_view* const tag);
	static std::string                      from_string_view(std::string_view in, const std::string* const tag);
	static bool                             from_string_view(std::string_view in, const bool* const tag);
	static boost::gregorian::date           from_string_view(std::string_view in, const boost::gregorian::date* const tag);
	static boost::posix_time::time_duration from_string_view(std::string_view in, const boost::posix_time::time_duration* const tag);
	static boost::posix_time::ptime         from_string_view(std::string_view in, const boost::posix_time::ptime* const tag);
	static date                             from_string_view(std::string_view in, const date* const tag);
	static time_of_day                      from_string_view(std::string_view in, const time_of_day* const tag);
	static time_point                       from_string_view(std::string_view in, const time_point* const tag);
	static boost::uuids::uuid               from_string_view(std::string_view in, const boost::uuids::uuid* const tag);

	template<typename T>
	static std::enable_if_t<std::is_scalar_v<T> && !std::is_enum_v<T>, T> from_string_view(std::string_view in, const T* const)
	{
		return conversion::string_to_number<T>(in);
	}

	template<typename T>
	static std::enable_if_t<std::is_enum_v<T> && boost::json::is_described_enum<T>::value, T> from_string_view(std::string_view in,
	                                                                                                           const T* const)
	{
		T e{};
		if (boost::describe::enum_from_string(std::string{ in }.c_str(), e))
		{
			return e;
		}
		else
		{
			ZOO_THROW_EXCEPTION(std::invalid_argument{ fmt::format("'{}' is not a valid `{}`", in, demangled_type_name<T>()) });
		}
	}

	template<typename T>
	static std::optional<T> from_string_view(std::string_view in, const std::optional<T>* const)
	{
		return std::make_optional<T>(from_string_view(in, static_cast<T*>(0)));
	}

	template<typename T>
	static boost::optional<T> from_string_view(std::string_view in, const boost::optional<T>* const)
	{
		return boost::make_optional(from_string_view(in, static_cast<T*>(0)));
	}
};

template<typename T>
concept ConvertibleFromStringView = requires(const std::string_view in, const T* const tag) {
	{ conversions::from_string_view(in, tag) } -> std::same_as<std::remove_cvref_t<T>>;
};

} // namespace spider
} // namespace zoo
