//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/mysql/config.h"
#include "zoo/squid/core/connection.h"

namespace zoo {
namespace squid {
namespace mysql {

class backend_connection;

// Convenience class to create a connection to an MySQL backend
// This class should be used if access to the native connection handle (MYSQL) is needed.
class ZOO_SQUID_MYSQL_API connection final : public squid::connection
{
	std::shared_ptr<backend_connection> backend_;

public:
	explicit connection(std::string_view connection_info);

	connection(const connection&)            = delete;
	connection(connection&& src)             = default;
	connection& operator=(const connection&) = delete;
	connection& operator=(connection&&)      = default;

	/// Get the backend
	/// The backend provides a getter for the native connection handle (MYSQL)
	const backend_connection& backend() const;
};

} // namespace mysql
} // namespace squid
} // namespace zoo
