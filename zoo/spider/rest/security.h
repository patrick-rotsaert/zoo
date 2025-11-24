//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/rest/isecurityscheme.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <string_view>

namespace zoo {
namespace spider {

using security_scopes = std::vector<std::string_view>;
using security        = std::vector<std::unordered_map<std::shared_ptr<isecurityscheme>, security_scopes>>;

} // namespace spider
} // namespace zoo
