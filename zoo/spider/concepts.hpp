//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/tag_invoke/optional.hpp"

#include <boost/json.hpp>
#include <type_traits>
#include <concepts>

namespace zoo {
namespace spider {

// template<typename T>
// concept ConvertibleToBoostJson = boost::json::is_described_class<T>::value;

// template<typename T>
// concept ConvertibleToBoostJson = !std::is_abstract_v<T> && requires(const std::remove_cvref_t<T>& t) {
// 	{ boost::json::value_from(t) } -> std::same_as<boost::json::value>;
// };

template<typename T>
concept ConvertibleToBoostJson = !std::is_abstract_v<T> && (requires(const std::remove_cvref_t<T>& t) {
	{ tag_invoke(boost::json::value_from_tag{}, t) } -> std::same_as<boost::json::value>;
} || boost::json::is_described_class<T>::value);

// template<typename T>
// concept ConvertibleFromBoostJson = !std::is_abstract_v<T> && requires(const boost::json::value& jv) {
// 	{ boost::json::value_to<std::remove_cvref_t<T>>(jv) } -> std::same_as<std::remove_cvref_t<T>>;
// };

template<typename T>
concept ConvertibleFromBoostJson = !std::is_abstract_v<T> && (requires(const boost::json::value& jv) {
	{ tag_invoke(boost::json::value_to_tag<std::remove_cvref_t<T>>{}, jv) } -> std::same_as<std::remove_cvref_t<T>>;
} || boost::json::is_described_class<T>::value);

} // namespace spider
} // namespace zoo
