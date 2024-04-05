//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"
#include <string>

namespace zoo {
namespace fs {
namespace sftp {

class ZOO_EXPORT ssh_identity final
{
public:
	std::string name;
	std::string pkey;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
