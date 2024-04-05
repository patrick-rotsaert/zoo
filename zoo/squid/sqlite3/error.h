//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/sqlite3/detail/sqlite3fwd.h"
#include "zoo/squid/core/error.h"
#include "zoo/common/api.h"

#include <optional>
#include <string>

namespace zoo {
namespace squid {
namespace sqlite {

class isqlite_api;

class ZOO_EXPORT error : public squid::error
{
	std::optional<int> ec_;

public:
	explicit error(const std::string& message);
	explicit error(isqlite_api& api, const std::string& message, sqlite3& connection);
	explicit error(isqlite_api& api, const std::string& message, int ec);

	const std::optional<int>& ec() const;
};

} // namespace sqlite
} // namespace squid
} // namespace zoo
