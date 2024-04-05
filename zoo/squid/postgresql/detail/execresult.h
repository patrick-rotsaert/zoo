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

struct exec_result final
{
	std::shared_ptr<PGresult> pgresult;
	int                       rows;
	int                       current_row;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
