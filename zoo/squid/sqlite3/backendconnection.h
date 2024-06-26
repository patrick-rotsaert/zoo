//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/sqlite3/config.h"
#include "zoo/squid/sqlite3/detail/sqlite3fwd.h"
#include "zoo/squid/core/ibackendconnection.h"

namespace zoo {
namespace squid {
namespace sqlite {

class isqlite_api;

class ZOO_SQUID_SQLITE_API backend_connection final : public ibackend_connection
{
	isqlite_api*             api_;
	std::shared_ptr<sqlite3> connection_;

public:
	/// @a connection_info must contain a path to a file
	/// or ":memory:" for an in-memory database.
	/// Files that do not exist will be created.
	explicit backend_connection(isqlite_api& api, const std::string& connection_info);

	backend_connection(const backend_connection&)            = delete;
	backend_connection(backend_connection&& src)             = default;
	backend_connection& operator=(const backend_connection&) = delete;
	backend_connection& operator=(backend_connection&&)      = default;

	std::unique_ptr<ibackend_statement> create_statement(std::string_view query) override;
	std::unique_ptr<ibackend_statement> create_prepared_statement(std::string_view query) override;
	void                                execute(const std::string& query) override;

	sqlite3& handle() const;
};

} // namespace sqlite
} // namespace squid
} // namespace zoo
