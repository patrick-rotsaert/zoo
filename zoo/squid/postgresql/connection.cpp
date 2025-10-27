//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/connection.h"
#include "zoo/squid/postgresql/backendconnection.h"
#include "zoo/squid/postgresql/backendconnectionfactory.h"

namespace zoo {
namespace squid {
namespace postgresql {

connection::connection(std::string_view connection_info)
    : squid::connection{ backend_connection_factory{}, connection_info }
    , backend_{ std::dynamic_pointer_cast<backend_connection>(this->squid::connection::backend()) }
{
}

connection::connection(ipq_api& api, std::string_view connection_info)
    : squid::connection{ backend_connection_factory{ api }, connection_info }
    , backend_{ std::dynamic_pointer_cast<backend_connection>(this->squid::connection::backend()) }
{
}

const backend_connection& connection::backend() const
{
	return *this->backend_;
}

void connection::async_exec(boost::asio::io_context&                                               io,
                            std::string_view                                                       query,
                            std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
                            async_exec_completion_handler                                          handler)
{
	this->backend_->run_async_exec(io, std::move(query), std::move(params), std::move(handler));
}

void connection::async_prepare(boost::asio::io_context& io, std::string_view query, async_prepare_completion_handler handler)
{
	this->backend_->run_async_prepare(io, std::move(query), std::move(handler));
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
