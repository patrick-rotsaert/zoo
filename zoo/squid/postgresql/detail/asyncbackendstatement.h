//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/config.h"
#include "zoo/squid/postgresql/async.h"
#include "zoo/squid/postgresql/detail/libpqfwd.h"
#include "zoo/squid/postgresql/detail/ipqapi.h"
#include "zoo/squid/core/parameter.h"

#include <boost/asio/io_context.hpp>

#include <memory>
#include <initializer_list>
#include <utility>
#include <string_view>

namespace zoo {
namespace squid {
namespace postgresql {

class ZOO_SQUID_POSTGRESQL_API async_backend_statement final
{
public:
	static void run(ipq_api*                                                               api,
	                std::shared_ptr<PGconn>                                                connection,
	                boost::asio::io_context&                                               io,
	                std::string_view                                                       query,
	                std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
	                async_completion_handler                                               handler);
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
