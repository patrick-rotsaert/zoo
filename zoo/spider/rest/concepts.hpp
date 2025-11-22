//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/aliases.h"
#include "zoo/common/misc/byte_string.h"

#include <boost/json/conversion.hpp>
#include <type_traits>
#include <concepts>
#include <stdexcept>
#include <string_view>

namespace zoo {
namespace spider {

template<typename T>
concept IsValidErrorType = std::is_class_v<T> && boost::json::is_described_class<T>::value && requires(const std::exception& e) {
	{ T::create(e) } -> std::same_as<T>;
} && requires(int ec, std::string m) {
	{ T::create(ec, m) } -> std::same_as<T>;
} && requires(const T& t) {
	{ t.status() } -> std::same_as<http::status>;
	{ t.status() } noexcept;
};

template<typename T>
concept HasStaticExampleMethod = std::is_class_v<T> && boost::json::is_described_class<T>::value && requires {
	{ T::example() } -> std::same_as<T>;
};

template<typename T>
concept HasStaticTypeNameMethod = std::is_class_v<T> && boost::json::is_described_class<T>::value && requires {
	{ T::type_name() } -> std::convertible_to<const std::string_view&>;
};

template<typename T>
concept IsStatusResult = requires {
	{ T::STATUS } -> std::convertible_to<http::status>;
	typename T::value_type;
} && requires(const T& t) {
	{ std::remove_cvref_t<decltype(t.result)>() } -> std::same_as<typename T::value_type>;
};

template<typename T>
concept IsBinaryContentContainer = requires {
	{ T::CONTENT_TYPE } -> std::convertible_to<const std::string_view&>;
} && requires(const T& t) {
	{ t.content_type() } -> std::same_as<std::string_view>;
	{ t.content_type() } noexcept;
} && requires(const T& t) {
	{ t.content() } -> std::same_as<byte_string_view>;
	{ t.content() } noexcept;
};

} // namespace spider
} // namespace zoo
