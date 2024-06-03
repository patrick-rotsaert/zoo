//
// Copyright (C) 2022 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/config.h"
#include "zoo/squid/postgresql/detail/libpqfwd.h"
#include "zoo/squid/core/ibackendconnection.h"

namespace zoo {
namespace squid {
namespace postgresql {

class ipq_api;

class ZOO_SQUID_POSTGRESQL_API backend_connection final : public ibackend_connection
{
	ipq_api*                api_;
	std::shared_ptr<PGconn> connection_;

public:
	/// @a connection_info must contain a valid PostgreSQL connection string
	explicit backend_connection(ipq_api* api, const std::string& connection_info);

	backend_connection(const backend_connection&)            = delete;
	backend_connection(backend_connection&& src)             = default;
	backend_connection& operator=(const backend_connection&) = delete;
	backend_connection& operator=(backend_connection&&)      = default;

	std::unique_ptr<ibackend_statement> create_statement(std::string_view query) override;
	std::unique_ptr<ibackend_statement> create_prepared_statement(std::string_view query) override;
	void                                execute(const std::string& query) override;

	PGconn& handle() const;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
