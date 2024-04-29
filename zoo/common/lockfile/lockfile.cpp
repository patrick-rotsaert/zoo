//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/common/lockfile/lockfile.h"

#if defined(_WIN32)
#include "zoo/common/lockfile/lockfile_impl_windows.hpp"
#else
#include "zoo/common/lockfile/lockfile_impl_posix.hpp"
#endif

namespace zoo {
namespace lockfile {

lockfile::lockfile(const std::filesystem::path& path)
    : pimpl_{ std::make_unique<impl>(path) }
{
}

lockfile::~lockfile() noexcept = default;

lockfile::lockfile(lockfile&&) noexcept = default;

lockfile& lockfile::operator=(lockfile&&) noexcept = default;

} // namespace lockfile
} // namespace zoo
