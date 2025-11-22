//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/logging/log_level.h"
#include "zoo/common/config.h"

#include <boost/assert/source_location.hpp>

#include <chrono>
#include <string_view>

namespace zoo {
namespace logging {

class ZOO_ZOOCOMMON_API ibackend
{
public:
	ibackend();
	virtual ~ibackend();

	virtual void log_message(const std::chrono::system_clock::time_point& time,
	                         const boost::source_location&                location,
	                         log_level                                    level,
	                         const std::string_view&                      message) = 0;
};

} // namespace logging
} // namespace zoo
