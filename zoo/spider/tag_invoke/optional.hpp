//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/json/conversion.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <boost/optional/optional.hpp>
#include <boost/version.hpp>

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

#if BOOST_VERSION < 108700
// Up to Boost 1.86, Boost.JSON only auto-detected std::optional as "optional-like"
// (via the internal boost::json::detail::is_optional trait). Mark boost::optional
// too, so missing/null members convert to boost::none instead of erroring.
// Boost 1.87 replaced this with the public boost::json::is_optional_like trait,
// which already recognises boost::optional, so no specialization is needed there.
namespace detail {

template<class T>
struct is_optional<boost::optional<T>> : std::true_type
{
};

} // namespace detail
#endif

} // namespace json
} // namespace boost
