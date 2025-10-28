//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/asyncexec.h"
#include "zoo/squid/postgresql/detail/ipqapi.h"
#include "zoo/squid/core/parameter.h"

#include <boost/asio/io_context.hpp>

#include <string>
#include <string_view>
#include <initializer_list>
#include <memory>

namespace zoo {
namespace squid {
namespace postgresql {

class backend_connection;
class postgresql_query;

class async_prepared_statement final
{
	ipq_api*                            api_;
	std::shared_ptr<backend_connection> connection_;
	boost::asio::io_context*            io_;
	std::unique_ptr<postgresql_query>   query_;
	std::string                         stmt_name_;

public:
	explicit async_prepared_statement(ipq_api*                            api,
	                                  std::shared_ptr<backend_connection> connection,
	                                  boost::asio::io_context&            io,
	                                  std::unique_ptr<postgresql_query>   query,
	                                  std::string                         stmt_name);
	~async_prepared_statement();

	async_prepared_statement(const async_prepared_statement&) = delete;
	async_prepared_statement(async_prepared_statement&& src);
	async_prepared_statement& operator=(const async_prepared_statement&) = delete;
	async_prepared_statement& operator=(async_prepared_statement&&);

	void async_exec(std::initializer_list<std::pair<std::string_view, parameter_by_value>> params, async_exec_completion_handler handler);
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
