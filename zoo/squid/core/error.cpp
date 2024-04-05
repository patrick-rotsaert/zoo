//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/core/error.h"

namespace zoo {
namespace squid {

error::error(const std::string& message)
    : std::runtime_error{ message }
{
}

} // namespace squid
} // namespace zoo
