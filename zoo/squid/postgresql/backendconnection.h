//
// Copyright (C) 2022 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/config.h"
#include "zoo/squid/postgresql/asyncexec.h"
#include "zoo/squid/postgresql/asyncprepare.h"
#include "zoo/squid/postgresql/detail/libpqfwd.h"
#include "zoo/squid/core/ibackendconnection.h"
#include "zoo/squid/core/parameter.h"

#include <boost/asio/io_context.hpp>

#include <initializer_list>
#include <utility>

namespace zoo {
namespace squid {
namespace postgresql {

class ipq_api;
class async_backend;
class postgresql_query;

class ZOO_SQUID_POSTGRESQL_API backend_connection final : public ibackend_connection,
                                                          public std::enable_shared_from_this<backend_connection>
{
	ipq_api*                api_;
	std::shared_ptr<PGconn> connection_;

public:
	/// @a connection_info must contain a valid PostgreSQL connection string
	explicit backend_connection(ipq_api* api, std::string_view connection_info);

	backend_connection(const backend_connection&)            = delete;
	backend_connection(backend_connection&& src)             = default;
	backend_connection& operator=(const backend_connection&) = delete;
	backend_connection& operator=(backend_connection&&)      = default;

	std::unique_ptr<ibackend_statement> create_statement(std::string_view query) override;
	std::unique_ptr<ibackend_statement> create_prepared_statement(std::string_view query) override;
	void                                execute(const std::string& query) override;

	void run_async_exec(boost::asio::io_context&                                               io,
	                    std::string_view                                                       query,
	                    std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
	                    async_exec_completion_handler                                          handler);
	void run_async_prepare(boost::asio::io_context& io, std::string_view query, async_prepare_completion_handler handler);
	void run_async_exec_prepared(boost::asio::io_context&                                               io,
	                             const postgresql_query&                                                query,
	                             std::string_view                                                       stmt_name,
	                             std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
	                             async_exec_completion_handler                                          handler);

	std::shared_ptr<PGconn> native_connection() const;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
