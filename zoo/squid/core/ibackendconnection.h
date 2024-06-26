//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/core/config.h"

#include <memory>
#include <string_view>

namespace zoo {
namespace squid {

class ibackend_statement;

/// Interface for a backend connection
class ZOO_SQUID_CORE_API ibackend_connection
{
public:
	virtual ~ibackend_connection() noexcept;

	virtual std::unique_ptr<ibackend_statement> create_statement(std::string_view query)          = 0;
	virtual std::unique_ptr<ibackend_statement> create_prepared_statement(std::string_view query) = 0;
	virtual void                                execute(const std::string& query)                 = 0;
};

} // namespace squid
} // namespace zoo
