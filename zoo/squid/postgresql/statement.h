//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/config.h"
#include "zoo/squid/postgresql/detail/libpqfwd.h"
#include "zoo/squid/postgresql/detail/ipqapifwd.h"
#include "zoo/squid/core/ibackendstatement.h"

#include <memory>

namespace zoo {
namespace squid {
namespace postgresql {

class ZOO_SQUID_POSTGRESQL_API statement final : public ibackend_statement
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	statement(ipq_api* api, std::shared_ptr<PGconn> connection, std::string_view query, bool reuse_statement);
	~statement() noexcept;

	statement(statement&&);
	statement& operator=(statement&&);

	statement(const statement&)            = delete;
	statement& operator=(const statement&) = delete;

	void execute(const std::map<std::string, parameter>& parameters, const std::vector<result>& results) override;
	void execute(const std::map<std::string, parameter>& parameters, const std::map<std::string, result>& results) override;

	bool fetch() override;

	std::size_t field_count() override;
	std::string field_name(std::size_t index) override;

	std::uint64_t affected_rows() override;

	static void execute(ipq_api* api, PGconn& connection, const std::string& query);
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
