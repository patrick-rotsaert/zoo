//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"
#include "zoo/fs/core/fspath.h"

namespace zoo {
namespace fs {

class ZOO_EXPORT source final
{
public:
	fspath orig_path;
	fspath current_path; // Differs from `orig_path` after moving the file (see operations.h)

	explicit source(const fspath& path);
};

} // namespace fs
} // namespace zoo
