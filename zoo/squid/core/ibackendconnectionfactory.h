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

class ibackend_connection;

/// Interface for a backend connection factory
class ZOO_SQUID_CORE_API ibackend_connection_factory
{
public:
	virtual ~ibackend_connection_factory() noexcept;

	virtual std::shared_ptr<ibackend_connection> create_backend_connection(std::string_view connection_info) const = 0;
};

} // namespace squid
} // namespace zoo
