//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/sqlite3/config.h"
#include "zoo/squid/core/connection.h"

namespace zoo {
namespace squid {
namespace sqlite {

class backend_connection;
class isqlite_api;

class ZOO_SQUID_SQLITE_API connection final : public squid::connection
{
	std::shared_ptr<backend_connection> backend_;

public:
	explicit connection(std::string_view connection_info);
	explicit connection(isqlite_api& api, std::string_view connection_info);

	connection(const connection&)            = delete;
	connection(connection&& src)             = default;
	connection& operator=(const connection&) = delete;
	connection& operator=(connection&&)      = default;

	/// Get the backend
	/// The backend provides a getter for the native connection handle (sqlite3)
	const backend_connection& backend() const;
};

} // namespace sqlite
} // namespace squid
} // namespace zoo
