//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/postgresql/asyncerror.h"
#include "zoo/squid/postgresql/resultset.h"

#include <functional>
#include <variant>

namespace zoo {
namespace squid {
namespace postgresql {

using async_result             = std::variant<resultset, async_error>;
using async_completion_handler = std::function<void(async_result)>;

} // namespace postgresql
} // namespace squid
} // namespace zoo
