//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/aliases.h"

namespace zoo {
namespace spider {

struct status_utility
{
	static http::status success_status_for_method(verb method)
	{
		return method == verb::post ? http::status::created : http::status::ok;
	}
};

} // namespace spider
} // namespace zoo
