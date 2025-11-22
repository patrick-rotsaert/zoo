//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <algorithm>

namespace zoo {

template<size_t N>
struct compile_time_string
{
	constexpr compile_time_string(const char (&str)[N])
	{
		std::copy_n(str, N, value);
	}
	char value[N]{};
};

template<compile_time_string cts>
constexpr auto operator""_cts()
{
	return cts;
}

} // namespace zoo
