//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/aliases.h"
#include "zoo/common/api.h"

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/empty_body.hpp>

namespace zoo {
namespace spider {

ZOO_EXPORT http::response<http::string_body>&& log_response(http::response<http::string_body>&& res);
ZOO_EXPORT http::response<http::empty_body>&& log_response(http::response<http::empty_body>&& res);

} // namespace spider
} // namespace zoo
