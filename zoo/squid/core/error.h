//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"

#include <stdexcept>
#include <string>

namespace zoo {
namespace squid {

/// Exception class
class ZOO_EXPORT error : public std::runtime_error
{
public:
	explicit error(const std::string& message);
};

} // namespace squid
} // namespace zoo
