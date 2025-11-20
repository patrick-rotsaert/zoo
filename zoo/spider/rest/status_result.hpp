//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/aliases.h"

#include <type_traits>

namespace zoo {
namespace spider {

template<http::status Status, typename T>
struct status_result
{
	static constexpr auto STATUS = Status;

	using value_type = std::decay_t<T>;
	value_type result;

	status_result(value_type&& result)
	    : result{ std::move(result) }
	{
	}
};

} // namespace spider
} // namespace zoo
