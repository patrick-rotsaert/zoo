//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/logging/ibackend.h"
#include "zoo/common/config.h"

#include <fmt/format.h>
#include <boost/assert/source_location.hpp>

#include <memory>
#include <chrono>

namespace zoo {
namespace logging {

class ZOO_COMMON_API logging final
{
public:
	static std::unique_ptr<ibackend> backend;

	// Not thread safe
	static void set_backend(std::unique_ptr<ibackend>&& backend);
};

} // namespace logging
} // namespace zoo

// Macro for logging using a fmtlib format string
#undef zlog
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

// Macros for filtering logging code at compile time
#undef ZOO_MIN_LOG
#define ZOO_MIN_LOG(minlvl, lvl, ...)                                                                                                      \
	do                                                                                                                                     \
	{                                                                                                                                      \
		if constexpr (::zoo::logging::log_level::lvl >= ::zoo::logging::log_level::minlvl)                                                 \
		{                                                                                                                                  \
			zlog(lvl, __VA_ARGS__);                                                                                                        \
		}                                                                                                                                  \
	} while (false)

// The default ZOO_LOGGING_LEVEL is defined in zoo/common/config.h but can be overridden by defining it prior to including this file, e.g.
// ```
// #define ZOO_LOGGING_LEVEL info
// ```
// will only compile calls to ZOO_LOG(info, ...) and higher levels.
#undef ZOO_LOG
#ifdef ZOO_LOGGING_LEVEL
#define ZOO_LOG(lvl, ...) ZOO_MIN_LOG(ZOO_LOGGING_LEVEL, lvl, __VA_ARGS__)
#else
#define ZOO_LOG(lvl, ...) ZOO_MIN_LOG(off, lvl, __VA_ARGS__)
#endif
