//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <variant>
#include <type_traits>

namespace zoo {

template<typename T>
struct is_variant : std::false_type
{
};

template<typename... Args>
struct is_variant<std::variant<Args...>> : std::true_type
{
};

template<typename T>
inline constexpr bool is_variant_v = is_variant<T>::value;

} // namespace zoo
