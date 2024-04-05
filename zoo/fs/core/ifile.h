//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"
#include <cstddef>

namespace zoo {
namespace fs {

class ZOO_EXPORT ifile
{
public:
	virtual ~ifile() noexcept;

	virtual std::size_t read(void* buf, std::size_t count)        = 0;
	virtual std::size_t write(const void* buf, std::size_t count) = 0;
};

} // namespace fs
} // namespace zoo
