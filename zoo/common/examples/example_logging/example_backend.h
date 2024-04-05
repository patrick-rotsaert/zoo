//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/logging/ibackend.h"

#include <mutex>

class example_backend final : public zoo::logging::ibackend
{
	std::mutex mutex_;

public:
	example_backend();
	~example_backend() override;

	void log_message(const std::chrono::system_clock::time_point& time,
	                 const boost::source_location&                location,
	                 zoo::logging::log_level                      level,
	                 const std::string_view&                      message) override;
};
