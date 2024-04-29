//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/local/local_access.h"
#include "zoo/fs/local/local_watcher.h"
#include "zoo/fs/core/noop_interruptor.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <boost/filesystem/operations.hpp>

#include <thread>

using namespace zoo::fs;
using namespace fmt::literals;

int main(int argc, char* argv[])
{
	auto dir = boost::filesystem::temp_directory_path();
	if (argc > 1)
	{
		dir = argv[1];
	}

	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("%L [%Y-%m-%d %H:%M:%S.%f](%t) %^%v%$ [%s:%#]");

	zlog(info, "application started");

	auto access  = local::access(std::make_shared<noop_interruptor>());
	auto watcher = access.create_watcher(dir);
	for (;;)
	{
		auto entries = watcher->watch();
		for (const auto& entry : entries)
		{
			zlog(info, "entry: {}", entry.name);
		}
	}

	return EXIT_SUCCESS;
}
