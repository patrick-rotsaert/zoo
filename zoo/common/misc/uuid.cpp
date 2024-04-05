//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/common/misc/uuid.h"
#include <boost/uuid/uuid_generators.hpp>

namespace zoo {

boost::uuids::uuid uuid::generate()
{
	thread_local auto gen = boost::uuids::random_generator{};
	return gen();
}

} // namespace zoo
