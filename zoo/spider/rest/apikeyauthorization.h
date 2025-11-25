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

namespace zoo {
namespace spider {

class ZOO_SPIDER_API api_key_authorization : public isecurityscheme
{
public:
	enum class source
	{
		header,
		query
	};

	explicit api_key_authorization(std::string_view scheme_name, source in, std::string_view name, std::string key);
	~api_key_authorization() override;

	std::string_view                 scheme_name() const override;
	boost::json::object              scheme() const override;
	std::expected<void, std::string> verify(request& req, const url_view& url, const std::vector<std::string_view>& scopes) const override;
	std::optional<std::string>       challenge() const override;

private:
	std::string_view scheme_name_;
	source           in_;
	std::string_view name_;
	std::string      key_;
};

} // namespace spider
} // namespace zoo
