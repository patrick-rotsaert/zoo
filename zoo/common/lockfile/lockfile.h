//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"

#include <filesystem>
#include <memory>

namespace zoo {
namespace lockfile {

class ZOO_EXPORT lockfile final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	lockfile(const std::filesystem::path& path);
	~lockfile() noexcept;

	lockfile(lockfile&&) noexcept;
	lockfile& operator=(lockfile&&) noexcept;

	lockfile(const lockfile&) = delete;
	lockfile& operator=(const lockfile&) = delete;
};

} // namespace lockfile
} // namespace zoo
