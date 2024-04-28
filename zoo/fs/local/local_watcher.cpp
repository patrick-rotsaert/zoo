//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/local/local_watcher.h"

#if defined(_WIN32)
#include "zoo/fs/local/local_watcher_impl_windows.hpp"
#else
#include "zoo/fs/local/local_watcher_impl_posix.hpp"
#endif

namespace zoo {
namespace fs {
namespace local {

watcher::watcher(const fspath& dir)
    : pimpl_{ std::make_unique<impl>(dir) }
{
}

watcher::~watcher() noexcept = default;

watcher::watcher(watcher&&) noexcept = default;

watcher& watcher::operator=(watcher&&) noexcept = default;

std::vector<direntry> watcher::watch()
{
	return this->pimpl_->watch();
}

void watcher::cancel()
{
	return this->pimpl_->cancel();
}

} // namespace local
} // namespace fs
} // namespace zoo
