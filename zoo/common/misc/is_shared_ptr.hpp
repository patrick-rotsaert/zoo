//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <type_traits>
#include <memory>

namespace zoo {

template<typename T, typename Enable = void>
struct is_shared_ptr final : std::false_type
{
};

template<typename T>
struct is_shared_ptr<std::shared_ptr<T>> final : std::true_type
{
};

template<typename T>
inline constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

} // namespace zoo
