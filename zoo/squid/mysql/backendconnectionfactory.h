//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/mysql/config.h"
#include "zoo/squid/core/ibackendconnectionfactory.h"

namespace zoo {
namespace squid {
namespace mysql {

class ZOO_SQUID_MYSQL_API backend_connection_factory final : public ibackend_connection_factory
{
	std::shared_ptr<ibackend_connection> create_backend_connection(std::string_view connection_info) const override;

public:
	backend_connection_factory()                                             = default;
	backend_connection_factory(const backend_connection_factory&)            = delete;
	backend_connection_factory(backend_connection_factory&& src)             = default;
	backend_connection_factory& operator=(const backend_connection_factory&) = delete;
	backend_connection_factory& operator=(backend_connection_factory&&)      = default;
};

} // namespace mysql
} // namespace squid
} // namespace zoo
