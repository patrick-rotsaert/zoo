//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/misc/cts.hpp"

#include <string>
#include <string_view>
#include <cstdint>

#include <algorithm>
#include <optional>

namespace zoo {
namespace spider {

template<compile_time_string ContentType, typename CharT>
class content_container
{
	static_assert(sizeof(CharT) == 1, "Invalid char type");

public:
	using char_type        = CharT;
	using string_type      = std::basic_string<char_type, std::char_traits<char_type>>;
	using string_view_type = std::basic_string_view<char_type, std::char_traits<char_type>>;

	static constexpr std::string_view CONTENT_TYPE = ContentType.value;

	static content_container create(std::string content_type, string_type content)
	{
		content_container cont{};
		cont.value_        = value{ std::move(content_type), std::move(content) };
		cont.content_type_ = cont.value_->content_type;
		cont.content_      = cont.value_->content;
		return cont;
	}

	static content_container create_view(std::string_view content_type, string_view_type content)
	{
		content_container cont{};
		cont.content_type_ = content_type;
		cont.content_      = content;
		return cont;
	}

	std::string_view content_type() const noexcept
	{
		return content_type_;
	}

	string_view_type content() const noexcept
	{
		return content_;
	}

private:
	struct value
	{
		std::string content_type;
		string_type content;
	};
	std::optional<value> value_;
	std::string_view     content_type_;
	string_view_type     content_;
};

template<compile_time_string ContentType>
using string_content_container = content_container<ContentType, char>;

template<compile_time_string ContentType>
using binary_content_container = content_container<ContentType, unsigned char>;

using image_container = binary_content_container<"image/*"_cts>;

} // namespace spider
} // namespace zoo
