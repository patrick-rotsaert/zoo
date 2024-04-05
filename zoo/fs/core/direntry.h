//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/attributes.h"
#include "zoo/fs/core/fspath.h"
#include "zoo/common/api.h"
#include <string>
#include <optional>

namespace zoo {
namespace fs {

class ZOO_EXPORT direntry final
{
public:
	std::string           name;
	attributes            attr;
	std::optional<fspath> symlink_target;

	direntry()                           = default;
	direntry(const direntry&)            = default;
	direntry(direntry&& src)             = default;
	direntry& operator=(const direntry&) = default;
	direntry& operator=(direntry&&)      = default;
};

} // namespace fs
} // namespace zoo
