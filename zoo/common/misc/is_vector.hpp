//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <vector>
#include <type_traits>

namespace zoo {

template<typename T>
struct is_vector : std::false_type
{
};

template<typename U, typename Alloc>
struct is_vector<std::vector<U, Alloc>> : std::true_type
{
};

template<typename T>
inline constexpr bool is_vector_v = is_vector<T>::value;

} // namespace zoo
