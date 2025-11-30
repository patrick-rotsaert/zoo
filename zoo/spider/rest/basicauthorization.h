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
	using verification_callback = std::function<std::expected<auth_data, std::string>(std::string_view user, std::string_view pass)>;

public:
	explicit basic_authorization(std::string_view scheme_name, verification_callback callback, std::string challenge_realm);
	~basic_authorization() override;

	std::string_view    scheme_name() const override;
	boost::json::object scheme() const override;

	std::expected<auth_data, auth_error>
	verify(request& req, const url_view& url, const std::vector<std::string_view>& scopes) const override;

private:
	auth_error make_verification_error(std::string message) const;

	std::string_view      scheme_name_;
	verification_callback callback_;
	std::string           challenge_realm_;
};

} // namespace spider
} // namespace zoo
