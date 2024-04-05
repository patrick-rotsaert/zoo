//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/core/parameter.h"
#include "zoo/squid/core/result.h"
#include "zoo/common/api.h"

#include <map>
#include <vector>
#include <string>

namespace zoo {
namespace squid {

/// Interface for a backend statement
class ZOO_EXPORT ibackend_statement
{
public:
	virtual ~ibackend_statement() noexcept;

	virtual void execute(const std::map<std::string, parameter>& parameters, const std::vector<result>& results)           = 0;
	virtual void execute(const std::map<std::string, parameter>& parameters, const std::map<std::string, result>& results) = 0;
	virtual bool fetch()                                                                                                   = 0;

	virtual std::size_t field_count()                 = 0;
	virtual std::string field_name(std::size_t index) = 0;

	virtual std::uint64_t affected_rows() = 0;
};

} // namespace squid
} // namespace zoo
