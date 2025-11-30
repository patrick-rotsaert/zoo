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

class ZOO_SPIDER_API bearer_authorization : public isecurityscheme
{
	using verification_callback = std::function<std::expected<auth_data, auth_error>(const bearer_authorization&, std::string_view token)>;

public:
	explicit bearer_authorization(std::string_view scheme_name, verification_callback callback, std::string challenge_realm);
	~bearer_authorization() override;

	std::string_view    scheme_name() const override;
	boost::json::object scheme() const override;

	std::expected<auth_data, auth_error>
	verify(request& req, const url_view& url, const std::vector<std::string_view>& scopes) const override;

	auth_error make_verification_error(std::string message, std::vector<std::pair<std::string_view, std::string>> auth_params) const;

private:
	std::string_view      scheme_name_;
	verification_callback callback_;
	std::string           challenge_realm_;
};

} // namespace spider
} // namespace zoo
