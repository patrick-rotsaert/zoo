//
// Copyright (C) 2022 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/detail/libpqfwd.h"
#include "zoo/squid/core/ibackendconnection.h"
#include "zoo/common/api.h"

namespace zoo {
namespace squid {
namespace postgresql {

class ZOO_EXPORT backend_connection final : public ibackend_connection
{
	std::shared_ptr<PGconn> connection_;

	std::unique_ptr<ibackend_statement> create_statement(std::string_view query) override;
	std::unique_ptr<ibackend_statement> create_prepared_statement(std::string_view query) override;
	void                                execute(const std::string& query) override;

public:
	/// @a connection_info must contain a valid PostgreSQL connection string
	explicit backend_connection(const std::string& connection_info);

	backend_connection(const backend_connection&)            = delete;
	backend_connection(backend_connection&& src)             = default;
	backend_connection& operator=(const backend_connection&) = delete;
	backend_connection& operator=(backend_connection&&)      = default;

	PGconn& handle() const;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
