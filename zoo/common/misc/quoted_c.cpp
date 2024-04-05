//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "quoted_c.h"

namespace zoo {

std::string quoted_c(std::string_view data)
{
	auto result = std::string{};
	result.resize(2 + 4 * data.length());
	auto out = &result[0];
	*out++   = '"';
	for (auto c : data)
	{
		switch (c)
		{
		case '"':
			*out++ = '\\';
			*out++ = '"';
			break;
		case '\\':
			*out++ = '\\';
			*out++ = '\\';
			break;
		case '\a':
			*out++ = '\\';
			*out++ = 'a';
			break;
		case '\b':
			*out++ = '\\';
			*out++ = 'b';
			break;
		case '\f':
			*out++ = '\\';
			*out++ = 'f';
			break;
		case '\n':
			*out++ = '\\';
			*out++ = 'n';
			break;
		case '\r':
			*out++ = '\\';
			*out++ = 'r';
			break;
		case '\t':
			*out++ = '\\';
			*out++ = 't';
			break;
		case '\v':
			*out++ = '\\';
			*out++ = 'v';
			break;
		case '\0':
			*out++ = '\\';
			*out++ = '0';
			break;
		default:
			if (c >= 0x20 && c < 0x7f)
			{
				*out++ = c;
			}
			else
			{
				static const char hexdigits[] = "0123456789ABCDEF";
				*out++                        = '\\';
				*out++                        = 'x';
				*out++                        = hexdigits[(c >> 4) & 15];
				*out++                        = hexdigits[(c >> 0) & 15];
			}
		}
	}
	*out++ = '"';
	result.resize(out - result.data());
	return result;
}

} // namespace zoo
