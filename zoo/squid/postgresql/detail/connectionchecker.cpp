//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/detail/connectionchecker.h"
#include "zoo/squid/postgresql/error.h"

#include "zoo/common/misc/throw_exception.h"

#include <cassert>

#include <libpq-fe.h>

namespace zoo {
namespace squid {
namespace postgresql {

PGconn* connection_checker::check(PGconn* connection)
{
	assert(connection);
	if (CONNECTION_OK != PQstatus(connection))
	{
		PQreset(connection);
		if (CONNECTION_OK != PQstatus(connection))
		{
			ZOO_THROW_EXCEPTION(error{ "PQreset failed", *connection });
		}
	}
	return connection;
}

PGconn* connection_checker::check(std::shared_ptr<PGconn> connection)
{
	return connection_checker::check(connection.get());
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
