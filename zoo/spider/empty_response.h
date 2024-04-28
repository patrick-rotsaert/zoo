//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/message.h"

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/empty_body.hpp>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API empty_response final
{
	using empty = http::response<http::empty_body>;

public:
	static empty create(const request& req, http::status status);
	static empty create(http::status status);
};

} // namespace spider
} // namespace zoo
