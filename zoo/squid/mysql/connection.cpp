//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/mysql/connection.h"
#include "zoo/squid/mysql/backendconnection.h"
#include "zoo/squid/mysql/backendconnectionfactory.h"

namespace zoo {
namespace squid {
namespace mysql {

connection::connection(std::string_view connection_info)
    : squid::connection{ backend_connection_factory{}, connection_info }
    , backend_{ std::dynamic_pointer_cast<backend_connection>(this->squid::connection::backend()) }
{
}

const backend_connection& connection::backend() const
{
	return *this->backend_;
}

} // namespace mysql
} // namespace squid
} // namespace zoo
