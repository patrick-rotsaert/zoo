//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/json/conversion.hpp>
#include <boost/json/detail/value_from.hpp>
#include <boost/json/detail/value_to.hpp>
#include <boost/optional/optional.hpp>

#include <type_traits>

namespace boost {
namespace json {

template<class T, class Ctx>
void tag_invoke(value_from_tag, value& jv, boost::optional<T> const& from, Ctx const& ctx)
{
	if (from)
	{
		value_from(*from, ctx, jv);
	}
	else
	{
		jv = nullptr;
	}
}

template<class T, class Ctx>
void tag_invoke(value_from_tag, value& jv, boost::optional<T>&& from, Ctx const& ctx)
{
	if (from)
	{
		value_from(std::move(*from), ctx, jv);
	}
	else
	{
		jv = nullptr;
	}
}

inline void tag_invoke(value_from_tag, value& jv, boost::none_t)
{
	// do nothing
	BOOST_ASSERT(jv.is_null());
	(void)jv;
}

template<typename T>
boost::optional<T> tag_invoke(const value_to_tag<boost::optional<T>>&, const value& in)
{
	if (in.is_null())
	{
		return boost::none;
	}
	else
	{
		return value_to<T>(in);
	}
}

namespace detail {

template<class T>
struct is_optional<boost::optional<T>> : std::true_type
{
};

} // namespace detail

} // namespace json
} // namespace boost
