//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "aliases.h"

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

namespace zoo {
namespace spider {

using request  = http::request<http::string_body>;
using response = http::response<http::string_body>;

} // namespace spider
} // namespace zoo
