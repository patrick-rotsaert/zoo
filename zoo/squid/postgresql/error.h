//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/config.h"
#include "zoo/squid/postgresql/detail/libpqfwd.h"
#include "zoo/squid/core/error.h"

#include <optional>
#include <string>

namespace zoo {
namespace squid {
namespace postgresql {

class ipq_api;

class ZOO_SQUID_POSTGRESQL_API error : public squid::error
{
	std::optional<std::string> sql_state_;

public:
	explicit error(const std::string& message);
	explicit error(ipq_api* api, const std::string& message, const PGconn& connection);
	explicit error(ipq_api* api, const std::string& message, const PGconn& connection, const PGresult& result);

	const std::optional<std::string>& sql_state() const;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
