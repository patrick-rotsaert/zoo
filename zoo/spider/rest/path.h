//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/aliases.h"

#include <vector>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API path final
{
public:
	explicit path();
	explicit path(string_view v);
	explicit path(std::vector<string_view> segments);

	path(path&&) noexcept;
	path& operator=(path&&) noexcept;

	path(const path&);
	path& operator=(const path&);

	const std::vector<string_view>& segments() const;
	std::string                     to_string() const;

	path& operator/=(const path& p);
	path  operator/(const path& p);

	path& operator/=(string_view v);
	path  operator/(string_view v);

private:
	std::vector<string_view> segments_;
};

} // namespace spider
} // namespace zoo
