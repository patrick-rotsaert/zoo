//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/iaccess.h"
#include "zoo/fs/core/iwatcher.h"
#include "zoo/fs/core/iinterruptor.h"
#include "zoo/fs/core/fspath.h"
#include "zoo/common/api.h"
#include <memory>
#include <cstddef>

namespace zoo {
namespace fs {
namespace sftp {

class ZOO_EXPORT watcher final : public iwatcher
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit watcher(const fspath&                 dir,
	                 std::uint32_t                 scan_interval_ms,
	                 std::shared_ptr<iaccess>      access,
	                 std::shared_ptr<iinterruptor> interruptor);
	~watcher() noexcept;

	watcher(watcher&&)            = default;
	watcher& operator=(watcher&&) = default;

	watcher(const watcher&)            = delete;
	watcher& operator=(const watcher&) = delete;

	std::vector<direntry> watch() override;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
