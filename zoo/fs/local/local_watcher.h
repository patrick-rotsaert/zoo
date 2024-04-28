//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/local/config.h"
#include "zoo/fs/core/iwatcher.h"
#include "zoo/fs/core/fspath.h"
#include <memory>

namespace zoo {
namespace fs {
namespace local {

class ZOO_FS_LOCAL_API watcher final : public iwatcher
{
private:
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit watcher(const fspath& dir);
	~watcher() noexcept;

	watcher(watcher&&) noexcept;
	watcher& operator=(watcher&&) noexcept;

	watcher(const watcher&)            = delete;
	watcher& operator=(const watcher&) = delete;

	std::vector<direntry> watch() override;
	void                  cancel() override;
};

} // namespace local
} // namespace fs
} // namespace zoo
