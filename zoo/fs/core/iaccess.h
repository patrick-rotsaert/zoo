//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/config.h"
#include "zoo/fs/core/fspath.h"
#include "zoo/common/compat.h"
#include <boost/system/api_config.hpp>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <fcntl.h>
#ifdef BOOST_WINDOWS_API
#include <io.h>
#endif

namespace zoo {
namespace fs {

class direntry;
class attributes;
class ifile;
class iwatcher;

class ZOO_FS_CORE_API iaccess
{
public:
	virtual ~iaccess() noexcept;

	virtual bool                      is_remote() const                                    = 0;
	virtual std::vector<direntry>     ls(const fspath& dir)                                = 0;
	virtual bool                      exists(const fspath& path)                           = 0;
	virtual std::optional<attributes> try_stat(const fspath& path)                         = 0;
	virtual attributes                stat(const fspath& path)                             = 0;
	virtual std::optional<attributes> try_lstat(const fspath& path)                        = 0;
	virtual attributes                lstat(const fspath& path)                            = 0;
	virtual void                      remove(const fspath& path)                           = 0; // if exists(path), it is removed
	virtual void                      mkdir(const fspath& path, bool parents)              = 0;
	virtual void                      rename(const fspath& oldpath, const fspath& newpath) = 0;
	virtual std::unique_ptr<ifile>    open(const fspath& path, int flags, mode_t mode)     = 0;

	virtual std::shared_ptr<iwatcher> create_watcher(const fspath& dir) = 0;
};

} // namespace fs
} // namespace zoo
