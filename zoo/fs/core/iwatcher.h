//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/config.h"
#include "zoo/fs/core/direntry.h"
#include <vector>
#include <memory>

namespace zoo {
namespace fs {

class ZOO_FS_CORE_API iwatcher
{
public:
	virtual ~iwatcher() noexcept;

	virtual std::vector<direntry> watch()  = 0;
	virtual void                  cancel() = 0;
};

} // namespace fs
} // namespace zoo
