//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/rest/pathfwd.h"
#include "zoo/spider/aliases.h"

#include <vector>
#include <string>
#include <optional>
#include <map>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API path_spec final
{
public:
	struct segment
	{
		string_view s;
		bool        is_parameter;

		bool operator==(const segment& other) const;
		bool operator!=(const segment& other) const;
	};

	explicit path_spec();
	explicit path_spec(string_view v);
	explicit path_spec(std::vector<segment> segments);

	path_spec(path_spec&&) noexcept;
	path_spec& operator=(path_spec&&) noexcept;

	path_spec(const path_spec&);
	path_spec& operator=(const path_spec&);

	const std::vector<segment>& segments() const;
	std::string                 to_string() const;

	using param_map = std::map<string_view, string_view>;
	std::optional<param_map> match(const path& p) const;

	path_spec& operator/=(const segment& s);
	path_spec  operator/(const segment& s);

	path_spec& operator/=(const path_spec& s);
	path_spec  operator/(const path_spec& s);

	path_spec& operator/=(string_view s);
	path_spec  operator/(string_view s);

	bool operator==(const path_spec& other) const;
	bool operator!=(const path_spec& other) const;

private:
	std::vector<segment> segments_;
};

} // namespace spider
} // namespace zoo
