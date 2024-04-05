//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/core/connection.h"
#include "zoo/common/api.h"

namespace zoo {
namespace squid {
namespace demo {

enum class Backend
{
	SQLITE3,
	POSTGRESQL,
	MYSQL
};

void ZOO_EXPORT demo_bindings(connection& connection);
void ZOO_EXPORT demo_field_info(connection& connection);
void ZOO_EXPORT demo_result_by_name(connection& connection);
void ZOO_EXPORT demo_query_stream(connection& connection);
void ZOO_EXPORT demo_bind_struct(connection& connection);
void ZOO_EXPORT demo_table_ops(connection& connection, Backend backend);

void ZOO_EXPORT demo_all(connection& connection, Backend backend);

} // namespace demo
} // namespace squid
} // namespace zoo
