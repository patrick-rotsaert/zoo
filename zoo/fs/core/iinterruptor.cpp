//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/core/iinterruptor.h"
#include "zoo/common/misc/throw_exception.h"

namespace zoo {
namespace fs {

iinterruptor::~iinterruptor() noexcept
{
}

void iinterruptor::throw_if_interrupted()
{
	if (is_interrupted())
	{
		ZOO_THROW_EXCEPTION(interrupted_exception{});
	}
}

} // namespace fs
} // namespace zoo
