//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/local/config.h"
#include "zoo/fs/core/iaccess.h"
#include "zoo/fs/core/iinterruptor.h"
#include <optional>

namespace zoo {
namespace fs {
namespace local {

class ZOO_FS_LOCAL_API access final : public iaccess
{
	std::shared_ptr<iinterruptor> interruptor_;

public:
	explicit access(std::shared_ptr<iinterruptor> interruptor);

	bool                      is_remote() const override;
	std::vector<direntry>     ls(const fspath& dir) override;
	bool                      exists(const fspath& path) override;
	std::optional<attributes> try_stat(const fspath& path) override;
	attributes                stat(const fspath& path) override;
	std::optional<attributes> try_lstat(const fspath& path) override;
	attributes                lstat(const fspath& path) override;
	void                      remove(const fspath& path) override;
	void                      mkdir(const fspath& path, bool parents) override;
	void                      rename(const fspath& oldpath, const fspath& newpath) override;
	std::unique_ptr<ifile>    open(const fspath& path, int flags, mode_t mode) override;
	std::shared_ptr<iwatcher> create_watcher(const fspath& dir) override;

	static direntry get_direntry(const fspath& path);
};

} // namespace local
} // namespace fs
} // namespace zoo
