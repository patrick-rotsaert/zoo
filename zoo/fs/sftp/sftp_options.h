//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/config.h"
#include <string>
#include <optional>
#include <cstdint>

namespace zoo {
namespace fs {
namespace sftp {

struct ZOO_FS_SFTP_API options final
{
	std::string                  host;
	std::optional<std::uint16_t> port = std::nullopt;
	std::string                  user;
	std::optional<std::string>   password = std::nullopt;

	// known-hosts policy
	bool allow_unknown_host_key = true;  // if true, then unknown host keys will be saved in the database.
	bool allow_changed_host_key = false; // dangerous!

	std::uint32_t watcher_scan_interval_ms = 5000;

	enum class ssh_log_level
	{
		NOLOG,
		WARNING,
		PROTOCOL,
		PACKET,
		FUNCTIONS
	};
	ssh_log_level ssh_logging_verbosity = ssh_log_level::NOLOG;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
