//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/config.h"
#include "zoo/squid/postgresql/asyncexec.h"
#include "zoo/squid/postgresql/asyncprepare.h"
#include "zoo/squid/postgresql/backendconnectionfwd.h"
#include "zoo/squid/postgresql/detail/ipqapifwd.h"
#include "zoo/squid/core/connection.h"
#include "zoo/squid/core/parameter.h"

#include <boost/asio/io_context.hpp>

#include <string_view>
#include <initializer_list>
#include <utility>

namespace zoo {
namespace squid {
namespace postgresql {

// Convenience class to create a connection to a PostgreSQL backend
// This class should be used if access to the native connection handle (PGconn) is needed.
class ZOO_SQUID_POSTGRESQL_API connection final : public squid::connection
{
	std::shared_ptr<backend_connection> backend_;

public:
	explicit connection(std::string_view connection_info);
	explicit connection(ipq_api& api, std::string_view connection_info);

	connection(const connection&)            = delete;
	connection(connection&& src)             = default;
	connection& operator=(const connection&) = delete;
	connection& operator=(connection&&)      = default;

	/// Get the backend
	/// The backend provides a getter for the native connection handle (PGconn)
	const backend_connection& backend() const;

	void async_exec(boost::asio::io_context&                                               io,
	                std::string_view                                                       query,
	                std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
	                async_exec_completion_handler                                          handler);

	void async_prepare(boost::asio::io_context& io, std::string_view query, async_prepare_completion_handler handler);
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
