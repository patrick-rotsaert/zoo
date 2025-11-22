//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/mysql/detail/mysqlfwd.h"
#include "zoo/squid/mysql/detail/queryfwd.h"
#include "zoo/squid/core/ibackendstatement.h"

#include <memory>
#include <string>

namespace zoo {
namespace squid {
namespace mysql {

class statement final : public ibackend_statement
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	statement(std::shared_ptr<MYSQL> connection, std::string_view query, bool reuse_statement);
	~statement() noexcept;

	statement(statement&&)            = default;
	statement& operator=(statement&&) = default;

	statement(const statement&)            = delete;
	statement& operator=(const statement&) = delete;

	void execute(const std::map<std::string, parameter>& parameters, const std::vector<result>& results) override;
	void execute(const std::map<std::string, parameter>& parameters, const std::map<std::string, result>& results) override;
	bool fetch() override;

	std::size_t field_count() override;
	std::string field_name(std::size_t index) override;

	std::uint64_t affected_rows() override;

	static void execute(MYSQL& connection, std::string_view query);

	MYSQL_STMT& handle() const;
};

} // namespace mysql
} // namespace squid
} // namespace zoo
