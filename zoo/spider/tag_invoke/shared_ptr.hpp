//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/json/conversion.hpp>
#include <boost/json/detail/value_from.hpp>
#include <boost/json/detail/value_to.hpp>

#include <type_traits>
#include <memory>

namespace boost {
namespace json {

template<class T, class Ctx>
void tag_invoke(value_from_tag, value& jv, std::shared_ptr<T> const& from, Ctx const& ctx)
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
void tag_invoke(value_from_tag, value& jv, std::shared_ptr<T>&& from, Ctx const& ctx)
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

inline void tag_invoke(value_from_tag, value& jv, std::nullptr_t)
{
	// do nothing
	BOOST_ASSERT(jv.is_null());
	(void)jv;
}

template<typename T>
std::shared_ptr<T> tag_invoke(const value_to_tag<std::shared_ptr<T>>&, const value& in)
{
	if (in.is_null())
	{
		return nullptr;
	}
	else
	{
		return std::make_shared<T>(value_to<T>(in));
	}
}

} // namespace json
} // namespace boost
