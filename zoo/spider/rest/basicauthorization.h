//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/rest/isecurityscheme.h"

#include <string>
#include <optional>
#include <functional>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API basic_authorization : public isecurityscheme
{
	using verification_callback = std::function<bool(std::string_view u, std::string_view p)>;

public:
	explicit basic_authorization(std::string_view                scheme_name,
	                             verification_callback           callback,
	                             std::string                     challenge_realm,
	                             std::optional<std::string_view> inject_username_header = std::nullopt);
	~basic_authorization() override;

	std::string_view                 scheme_name() const override;
	boost::json::object              scheme() const override;
	std::expected<void, std::string> verify(request& req, const url_view& url, const std::vector<std::string_view>& scopes) const override;
	std::optional<std::string>       challenge() const override;

private:
	std::string_view                scheme_name_;
	verification_callback           callback_;
	std::string                     challenge_realm_;
	std::optional<std::string_view> inject_username_header_;
};

} // namespace spider
} // namespace zoo
