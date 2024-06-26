//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/detail/libpqfwd.h"

#include <memory>

namespace zoo {
namespace squid {
namespace postgresql {

class ipq_api;

class connection_checker final
{
public:
	// must not return a nullptr
	static PGconn* check(ipq_api *api, PGconn* connection);

	// must not return a nullptr
	// connection must outlive the returned pointer
	static PGconn* check(ipq_api *api, std::shared_ptr<PGconn> connection);
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
