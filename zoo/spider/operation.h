//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/pathspec.h"
#include "zoo/spider/aliases.h"

#include <string_view>

namespace zoo {
namespace spider {

struct operation final
{
	verb             method;
	path_spec        path;
	std::string_view operation_id;
	std::string_view summary;
};

} // namespace spider
} // namespace zoo
