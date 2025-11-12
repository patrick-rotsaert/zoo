//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/path.h"

#include <boost/url/url_view.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <algorithm>

namespace zoo {
namespace spider {
namespace {

std::vector<string_view> split(string_view v)
{
	std::vector<string_view> segments{};
	boost::algorithm::split(segments, v, boost::is_any_of("/"), boost::token_compress_on);
	segments.erase(std::remove_if(segments.begin(), segments.end(), [](const string_view s) { return s.empty(); }), segments.end());
	return segments;
}

} // namespace

path::path() = default;

path::path(std::vector<string_view> segments)
    : segments_{ std::move(segments) }
{
}

path::path(string_view v)
    : path{ split(v) }
{
}

path::path(path&&) noexcept            = default;
path& path::operator=(path&&) noexcept = default;

path::path(const path&)            = default;
path& path::operator=(const path&) = default;

const std::vector<string_view>& path::segments() const
{
	return segments_;
}

path& path::operator/=(const path& p)
{
	segments_.insert(segments_.end(), p.segments_.cbegin(), p.segments_.cend());
	return *this;
}

path path::operator/(const path& other)
{
	auto p = *this;
	p /= other;
	return p;
}

path& path::operator/=(string_view v)
{
	return (*this /= path{ v });
}

path path::operator/(string_view v)
{
	return (*this / path{ v });
}

} // namespace spider
} // namespace zoo
