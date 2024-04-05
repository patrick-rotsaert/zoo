//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"
#include "zoo/fs/core/exceptions.h"
#include <chrono>

namespace zoo {
namespace fs {

struct ZOO_EXPORT interrupted_exception : public exception
{
};

class ZOO_EXPORT iinterruptor
{
public:
	virtual ~iinterruptor() noexcept;

	virtual void interrupt()                                               = 0;
	virtual bool is_interrupted()                                          = 0;
	virtual bool wait_for_interruption(std::chrono::milliseconds duration) = 0; // returns true if interrupted, false on timeout

	void throw_if_interrupted(); // throws interrupted_exception if is_interrupted()
};

} // namespace fs
} // namespace zoo
