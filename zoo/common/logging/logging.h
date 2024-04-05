//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/logging/ibackend.h"
#include "zoo/common/api.h"
#include "zoo/common/config.h"

#include <fmt/format.h>
#include <boost/assert/source_location.hpp>

#include <memory>
#include <chrono>

namespace zoo {
namespace logging {

class ZOO_EXPORT logging final
{
public:
	static std::unique_ptr<ibackend> backend;

	// Not thread safe
	static void set_backend(std::unique_ptr<ibackend>&& backend);
};

} // namespace logging
} // namespace zoo

// Macro for logging using a fmtlib format string
#define zlog(lvl, ...)                                                                                                                     \
	do                                                                                                                                     \
	{                                                                                                                                      \
		auto& backend = ::zoo::logging::logging::backend;                                                                                  \
		if (backend)                                                                                                                       \
		{                                                                                                                                  \
			backend->log_message(                                                                                                          \
			    std::chrono::system_clock::now(), BOOST_CURRENT_LOCATION, ::zoo::logging::log_level::lvl, fmt::format(__VA_ARGS__));       \
		}                                                                                                                                  \
	} while (false)

// Macros for eliding logging code at compile time
#undef ZOO_MIN_LOG
#define ZOO_MIN_LOG(minlvl, lvl, ...)                                                                                                      \
	do                                                                                                                                     \
	{                                                                                                                                      \
		if constexpr (::zoo::logging::log_level::lvl >= ::zoo::logging::log_level::minlvl)                                                 \
		{                                                                                                                                  \
			zlog(lvl, __VA_ARGS__);                                                                                                        \
		}                                                                                                                                  \
	} while (false)

#undef ZOO_LOG
#define ZOO_LOG(lvl, ...) ZOO_MIN_LOG(ZOO_LOGGING_LEVEL, lvl, __VA_ARGS__)
