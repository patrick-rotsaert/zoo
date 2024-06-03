//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/backendconnectionfactory.h"
#include "zoo/squid/postgresql/backendconnection.h"

#include "zoo/squid/postgresql/detail/pqapi.h"

namespace zoo {
namespace squid {
namespace postgresql {

std::shared_ptr<ibackend_connection> backend_connection_factory::create_backend_connection(std::string_view connection_info) const
{
	return std::make_shared<backend_connection>(this->api_, std::string{ connection_info });
}

backend_connection_factory::backend_connection_factory()
    : api_{ &pq_api::API }
{
}

backend_connection_factory::backend_connection_factory(ipq_api& api)
    : api_{ &api }
{
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
