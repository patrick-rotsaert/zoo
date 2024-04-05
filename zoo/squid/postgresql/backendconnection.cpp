//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/backendconnection.h"
#include "zoo/squid/postgresql/statement.h"
#include "zoo/squid/postgresql/error.h"

#include "zoo/squid/postgresql/detail/connectionchecker.h"

#include "zoo/common/misc/throw_exception.h"

#include <libpq-fe.h>

namespace zoo {
namespace squid {
namespace postgresql {

std::unique_ptr<ibackend_statement> backend_connection::create_statement(std::string_view query)
{
	return std::make_unique<statement>(this->connection_, query, false);
}

std::unique_ptr<ibackend_statement> backend_connection::create_prepared_statement(std::string_view query)
{
	return std::make_unique<statement>(this->connection_, query, true);
}

/* static */ void backend_connection::execute(const std::string& query)
{
	statement::execute(*connection_checker::check(this->connection_), query);
}

backend_connection::backend_connection(const std::string& connection_info)
    : connection_{ PQconnectdb(connection_info.c_str()), PQfinish }
{
	if (this->connection_)
	{
		if (CONNECTION_OK != PQstatus(this->connection_.get()))
		{
			ZOO_THROW_EXCEPTION(error{ "PQconnectdb failed", *this->connection_.get() });
		}
	}
	else
	{
		ZOO_THROW_EXCEPTION(error{ "PQconnectdb failed" });
	}
}

PGconn& backend_connection::handle() const
{
	return *this->connection_;
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
