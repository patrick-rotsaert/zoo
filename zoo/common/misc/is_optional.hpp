//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/optional/optional.hpp>

#include <type_traits>
#include <optional>

namespace zoo {

template<typename T, typename Enable = void>
struct is_optional final : std::false_type
{
};

template<typename T>
struct is_optional<std::optional<T>> final : std::true_type
{
};

template<typename T>
struct is_optional<boost::optional<T>> final : std::true_type
{
};

template<typename T>
inline constexpr bool is_optional_v = is_optional<T>::value;

} // namespace zoo
