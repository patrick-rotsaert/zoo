//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/aliases.h"

#include <string_view>
#include <cstdint>
#include <memory>

namespace zoo {
namespace spider {

// Accepts incoming connections and launches the sessions
class ZOO_SPIDER_API listener final
{
public:
	// Start accepting incoming connections
	static void run(net::io_context&                         ioc,
	                const std::string_view&                  address,
	                std::uint16_t                            port,
	                const std::shared_ptr<irequest_handler>& request_handler,
	                beast::error_code&                       ec);
};

} // namespace spider
} // namespace zoo
