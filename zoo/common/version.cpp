//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/common/version.h"

#include <cassert>

namespace zoo {

int version::number()
{
	return ZOO_VERSION_NUMBER;
}

int version::major()
{
	return ZOO_VERSION_MAJOR;
}

int version::minor()
{
	return ZOO_VERSION_MINOR;
}

int version::patch()
{
	return ZOO_VERSION_PATCH;
}

void version::check()
{
	assert(ZOO_VERSION_NUMBER == number());
}

} // namespace zoo
