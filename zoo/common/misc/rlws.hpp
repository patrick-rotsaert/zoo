#pragma once

#include <cstddef>   // For std::size_t
#include <algorithm> // For std::ranges::copy
#include <ranges>    // For std::ranges

namespace zoo {
namespace detail {

template<std::size_t N>
struct RemoveLeadingWhitespace
{
	char p[N]{};

	constexpr RemoveLeadingWhitespace(char const (&str)[N])
	{
		if (N <= 1)
		{
			p[0] = '\0';
			return;
		}

		const auto length = N - 1;

		std::size_t ii{};
		char        eol = '\n';

		if (str[0] == '\n')
		{
			ii = 1;
		}
		else if (str[0] == '\r')
		{
			if (length > 1 && str[1] == '\n')
			{
				ii = 2;
			}
			else
			{
				ii  = 1;
				eol = '\r';
			}
		}
		else
		{
			std::ranges::copy(str, str + length, p);
			p[length] = '\0';
			return;
		}

		if (length <= ii)
		{
			std::ranges::copy(str, str + length, p);
			p[length] = '\0';
			return;
		}

		const auto first = str[ii];
		if (first != '\t' && first != ' ')
		{
			std::ranges::copy(str, str + length, p);
			p[length] = '\0';
			return;
		}

		std::size_t count = 1;
		while (++ii < length)
		{
			if (str[ii] == first)
			{
				++count;
			}
			else
			{
				break;
			}
		}

		enum
		{
			COPYING,
			SKIPPING
		} state = COPYING;

		std::size_t oi = 0;
		std::size_t skipped{};
		for (; ii < length; ++ii)
		{
			const auto c = str[ii];
			if (state == COPYING)
			{
				p[oi++] = c;
				if (c == eol)
				{
					state   = SKIPPING;
					skipped = 0;
				}
			}
			else if (state == SKIPPING)
			{
				if (c != first || skipped++ == count)
				{
					state = COPYING;
					--ii;
				}
			}
		}

		p[oi] = '\0';
	}
};

} // namespace detail

/*
trimlws removes leading whitespace

Intended use is for raw string literals, e.g.
	constexpr auto query = trimlws(R"(
		SELECT
		  *
		FROM
		  "public"."foo"
		WHERE
		  bar = 'baz'
	)");

The result of the above example would be:
SELECT
  *
FROM
  "public"."foo"
WHERE
  bar = 'baz'
<<<END

The string must start with "\n", "\r\n" or "\r".
All subsequent SPACE or TAB characters are considered leading whitespace.
In the example above these are the TAB characters leading up to "SELECT".
Note: all leading whitespace characters on the second line should be all SPACE or all TAB, not mixed.
The counted leading whitespace characters are removed from all subsequent lines.

TODO: remove the trailing EOL
*/

template<std::size_t N>
constexpr auto rlws(const char (&str)[N])
{
	return zoo::detail::RemoveLeadingWhitespace<N>{ str }.p;
}

} // namespace zoo

template<zoo::detail::RemoveLeadingWhitespace A>
constexpr auto operator""_rlws()
{
	return A.p;
}
