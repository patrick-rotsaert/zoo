//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace zoo {
namespace spider {

// Tunables for an HTTP server connection. Defaults preserve the historical behaviour.
struct http_session_settings
{
	// Maximum size of a request body, in bytes. Requests with a larger body are rejected.
	std::uint64_t body_limit = 10000;

	// Per-request inactivity timeout.
	std::chrono::steady_clock::duration timeout = std::chrono::seconds{ 30 };

	// Maximum number of pipelined responses queued before the read loop is paused. Must be >= 1;
	// values below 1 are treated as 1.
	std::size_t pipeline_queue_limit = 8;
};

} // namespace spider
} // namespace zoo
