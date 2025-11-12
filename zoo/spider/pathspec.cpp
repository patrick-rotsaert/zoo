//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/pathspec.h"
#include "zoo/spider/path.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <algorithm>
#include <ranges>

namespace zoo {
namespace spider {
namespace {

std::vector<path_spec::segment> split(string_view v)
{
	std::vector<string_view> path{};
	boost::algorithm::split(path, v, boost::is_any_of("/"), boost::token_compress_on);
	path.erase(std::remove_if(path.begin(), path.end(), [](const string_view s) { return s.empty(); }), path.end());
	std::vector<path_spec::segment> segments{};
	std::transform(path.begin(), path.end(), std::back_inserter(segments), [](const string_view& v) {
		if (v.length() > 2u && v.front() == '{' && v.back() == '}')
		{
			return path_spec::segment{ v.substr(1u, v.length() - 2u), true };
		}
		else
		{
			return path_spec::segment{ v, false };
		}
	});
	return segments;
}

} // namespace

bool path_spec::segment::operator==(const segment& other) const
{
	return s == other.s && is_parameter == other.is_parameter;
}

bool path_spec::segment::operator!=(const segment& other) const
{
	return !(*this == other);
}

path_spec::path_spec() = default;

path_spec::path_spec(std::vector<segment> segments)
    : segments_{ std::move(segments) }
{
}

path_spec::path_spec(string_view v)
    : path_spec{ split(v) }
{
}

path_spec::path_spec(path_spec&&) noexcept            = default;
path_spec& path_spec::operator=(path_spec&&) noexcept = default;

path_spec::path_spec(const path_spec&)            = default;
path_spec& path_spec::operator=(const path_spec&) = default;

const std::vector<path_spec::segment>& path_spec::segments() const
{
	return segments_;
}

std::string path_spec::to_string() const
{
	std::string result{};
	for (const auto& segment : segments_)
	{
		if (!result.empty())
		{
			result.append("/");
		}
		if (segment.is_parameter)
		{
			result.append("{");
		}
		result.append(segment.s);
		if (segment.is_parameter)
		{
			result.append("}");
		}
	}
	return result;
}

std::optional<path_spec::param_map> path_spec::match(const path& p) const
{
	if (segments_.size() != p.segments().size())
	{
		return std::nullopt;
	}
	auto                               si = segments_.begin();
	auto                               pi = p.segments().begin();
	std::map<string_view, string_view> m{};
	for (; si != segments_.end();)
	{
		if (si->is_parameter)
		{
			m[si->s] = *pi;
		}
		else if (si->s != *pi)
		{
			return std::nullopt;
		}
		++si;
		++pi;
	}
	return m;
}

path_spec& path_spec::operator/=(const segment& s)
{
	segments_.push_back(s);
	return *this;
}

path_spec path_spec::operator/(const segment& s)
{
	auto p = *this;
	p /= s;
	return p;
}

path_spec& path_spec::operator/=(const path_spec& s)
{
	segments_.insert(segments_.end(), s.segments_.cbegin(), s.segments_.cend());
	return *this;
}

path_spec path_spec::operator/(const path_spec& s)
{
	auto p = *this;
	p /= s;
	return p;
}

path_spec& path_spec::operator/=(string_view s)
{
	return (*this /= path_spec{ s });
}

path_spec path_spec::operator/(string_view s)
{
	return (*this / path_spec{ s });
}

bool path_spec::operator==(const path_spec& other) const
{
	return segments_ == other.segments_;
}

bool path_spec::operator!=(const path_spec& other) const
{
	return !(*this == other);
}

} // namespace spider
} // namespace zoo
