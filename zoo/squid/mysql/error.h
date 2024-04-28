//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/mysql/config.h"
#include "zoo/squid/mysql/detail/mysqlfwd.h"
#include "zoo/squid/core/error.h"

#include <optional>
#include <string>

namespace zoo {
namespace squid {
namespace mysql {

class ZOO_SQUID_MYSQL_API error : public squid::error
{
	std::optional<unsigned int> ec_;

public:
	explicit error(const std::string& message);
	explicit error(const std::string& message, MYSQL& connection);
	explicit error(const std::string& message, MYSQL_STMT& stmt);

	const std::optional<unsigned int>& ec() const;
};

} // namespace mysql
} // namespace squid
} // namespace zoo
