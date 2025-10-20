//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/backendconnection.h"
#include "zoo/squid/postgresql/statement.h"
#include "zoo/squid/postgresql/error.h"

#include "zoo/squid/postgresql/detail/asyncbackendstatement.h"
#include "zoo/squid/postgresql/detail/ipqapi.h"
#include "zoo/squid/postgresql/detail/connectionchecker.h"

#include "zoo/common/misc/throw_exception.h"

#include <libpq-fe.h>

namespace zoo {
namespace squid {
namespace postgresql {

std::unique_ptr<ibackend_statement> backend_connection::create_statement(std::string_view query)
{
	return std::make_unique<statement>(this->api_, this->connection_, query, false);
}

std::unique_ptr<ibackend_statement> backend_connection::create_prepared_statement(std::string_view query)
{
	return std::make_unique<statement>(this->api_, this->connection_, query, true);
}

void backend_connection::execute(const std::string& query)
{
	statement::execute(this->api_, *connection_checker::check(this->api_, this->connection_), query);
}

backend_connection::backend_connection(ipq_api* api, std::string_view connection_info)
    : api_{ api }
    , connection_{ api->connectdb(std::string{ connection_info }.c_str()), [api](PGconn* conn) { api->finish(conn); } }
{
	if (this->connection_)
	{
		if (CONNECTION_OK != api->status(this->connection_.get()))
		{
			ZOO_THROW_EXCEPTION(error{ api, "PQconnectdb failed", *this->connection_.get() });
		}
	}
	else
	{
		ZOO_THROW_EXCEPTION(error{ "PQconnectdb failed" });
	}
}

void backend_connection::run_async_statement(boost::asio::io_context&                                               io,
                                             std::string_view                                                       query,
                                             std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
                                             async_completion_handler                                               handler)
{
	async_backend_statement::run(this->api_, this->connection_, io, std::move(query), std::move(params), std::move(handler));
}

std::shared_ptr<PGconn> backend_connection::native_connection() const
{
	return this->connection_;
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
