//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/aliases.h"
#include "zoo/spider/tracked_file.h"

#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/vector_body.hpp>
#include <boost/beast/http/message_generator.hpp>

#include <variant>
#include <cstdint>

namespace zoo {
namespace spider {

class response_wrapper final
{
	std::variant<http::response<http::basic_file_body<tracked_file>>,
	             http::response<http::file_body>,
	             http::response<http::string_body>,
	             http::response<http::buffer_body>,
	             http::response<http::vector_body<std::uint8_t>>,
	             http::response<http::empty_body>>
	    value_;

public:
	template<class R>
	response_wrapper(R&& r)
	    : value_{ std::move(r) }
	{
	}

	operator message_generator()
	{
		return std::visit([](auto&& arg) -> message_generator { return std::move(arg); }, this->value_);
	}

	void keep_alive(bool value)
	{
		std::visit([&](auto&& arg) { arg.keep_alive(value); }, this->value_);
	}

	void version(unsigned value)
	{
		std::visit([&](auto&& arg) { arg.version(value); }, this->value_);
	}
};

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
