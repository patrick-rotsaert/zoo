//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/config.h"
#include <boost/uuid/uuid.hpp>

namespace zoo {

class ZOO_COMMON_API uuid final
{
public:
	static boost::uuids::uuid generate();
};

} // namespace zoo
