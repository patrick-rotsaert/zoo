//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/sqlite3/backendconnectionfactory.h"
#include "zoo/squid/sqlite3/backendconnection.h"

#include "zoo/squid/sqlite3/detail/sqliteapi.h"

namespace zoo {
namespace squid {
namespace sqlite {

std::shared_ptr<ibackend_connection> backend_connection_factory::create_backend_connection(std::string_view connection_info) const
{
	return std::make_shared<backend_connection>(*this->api_, std::string{ connection_info });
}

backend_connection_factory::backend_connection_factory()
    : api_{ &sqlite_api::API }
{
}

backend_connection_factory::backend_connection_factory(isqlite_api& api)
    : api_{ &api }
{
}

} // namespace sqlite
} // namespace squid
} // namespace zoo
