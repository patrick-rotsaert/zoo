//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/core/result.h"

namespace zoo {
namespace squid {

const result::type& result::value() const noexcept
{
	return this->value_;
}

} // namespace squid
} // namespace zoo
