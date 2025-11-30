//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/rest/bearerauthorization.h"

#include <boost/date_time/posix_time/ptime.hpp>

#include <expected>
#include <string>

namespace demo {

using namespace zoo::spider;

struct BasicAuthData : public auth_data_base
{
	std::string userName;
};

struct BearerAuthData : public auth_data_base
{
	std::string              userName;
	boost::posix_time::ptime exp;

	static std::expected<auth_data, auth_error> verify(const bearer_authorization&, std::string_view token);

	std::string asToken() const;
};

} // namespace demo
