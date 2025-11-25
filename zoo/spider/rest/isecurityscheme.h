//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/message.h"

#include <boost/json.hpp>
#include <boost/url/url_view.hpp>

#include <string_view>
#include <string>
#include <vector>
#include <optional>
#include <expected>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API isecurityscheme
{
public:
	explicit isecurityscheme();
	virtual ~isecurityscheme();

	isecurityscheme(isecurityscheme&&)      = delete;
	isecurityscheme(const isecurityscheme&) = delete;

	isecurityscheme& operator=(isecurityscheme&&)      = delete;
	isecurityscheme& operator=(const isecurityscheme&) = delete;

	virtual std::string_view    scheme_name() const = 0;
	virtual boost::json::object scheme() const      = 0;

	virtual std::expected<void, std::string>
	verify(request& req, const url_view& url, const std::vector<std::string_view>& scopes) const = 0;

	virtual std::optional<std::string> challenge() const = 0;
};

} // namespace spider
} // namespace zoo
