//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/aliases.h"

#include <boost/beast/http/message.hpp>

#include <type_traits>

namespace zoo {
namespace spider {

template<typename T>
struct is_http_response : std::false_type
{
};

template<typename U>
struct is_http_response<http::response<U>> : std::true_type
{
};

template<typename T>
inline constexpr bool is_http_response_v = is_http_response<T>::value;

} // namespace spider
} // namespace zoo
