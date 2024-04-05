//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/core/noop_interruptor.h"

namespace zoo {
namespace fs {

void noop_interruptor::interrupt()
{
}

bool noop_interruptor::is_interrupted()
{
	return false;
}

bool noop_interruptor::wait_for_interruption(std::chrono::milliseconds /*duration*/)
{
	return false;
}

} // namespace fs
} // namespace zoo
