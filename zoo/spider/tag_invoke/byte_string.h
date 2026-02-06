//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/common/misc/byte_string.h"

#include <boost/json/conversion.hpp>

namespace boost::json {

void ZOO_SPIDER_API             tag_invoke(const boost::json::value_from_tag&, boost::json::value& out, const zoo::byte_string& in);
zoo::byte_string ZOO_SPIDER_API tag_invoke(const boost::json::value_to_tag<zoo::byte_string>&, const boost::json::value& in);

} // namespace boost::json
