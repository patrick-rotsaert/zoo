//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/detail/conversions.h"

#include "zoo/common/misc/throw_exception.h"

namespace zoo {
namespace squid {
namespace postgresql {

namespace {

byte_string::value_type hex_char_to_nibble(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	else if (c >= 'a' && c <= 'f')
	{
		return 0xa + c - 'a';
	}
	else if (c >= 'A' && c <= 'F')
	{
		return 0xa + c - 'A';
	}
	else
	{
		ZOO_THROW_EXCEPTION(std::runtime_error{ "illegal hex character" });
	}
}

template<class It>
void binary_to_hex_string_internal(It begin, It end, std::string& out)
{
	constexpr auto hex = "0123456789ABCDEF";

	out.resize(2 + 2 * std::distance(begin, end));
	auto p = out.data();
	*p++   = '\\';
	*p++   = 'x';
	for (auto it = begin; it < end; ++it)
	{
		*p++ = hex[*it >> 4];
		*p++ = hex[*it & 15];
	}
}

} // namespace

void hex_string_to_binary(std::string_view in, byte_string& out)
{
	if (!in.starts_with("\\x"))
	{
		ZOO_THROW_EXCEPTION(std::runtime_error{ "string does not start with \"\\x\"" });
	}

	if (in.length() % 2)
	{
		ZOO_THROW_EXCEPTION(std::runtime_error{ "string length is not a multiple of 2" });
	}

	out.resize((in.length() - 2) / 2);
	auto p = out.data();

	for (auto it = in.begin() + 2, end = in.end(); it != end;)
	{
		*p = (hex_char_to_nibble(*it++) << 4);
		*p++ |= hex_char_to_nibble(*it++);
	}
}

byte_string hex_string_to_binary(std::string_view in)
{
	byte_string result;
	hex_string_to_binary(in, result);
	return result;
}

void binary_to_hex_string(const unsigned char* begin, const unsigned char* end, std::string& out)
{
	return binary_to_hex_string_internal(begin, end, out);
}

std::string binary_to_hex_string(const unsigned char* begin, const unsigned char* end)
{
	std::string result;
	binary_to_hex_string(begin, end, result);
	return result;
}

void binary_to_hex_string(byte_string_view in, std::string& out)
{
	binary_to_hex_string_internal(in.begin(), in.end(), out);
}

std::string binary_to_hex_string(byte_string_view in)
{
	std::string result;
	binary_to_hex_string(in, result);
	return result;
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
