//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/config.h"
#include "zoo/fs/core/iaccess.h"
#include "zoo/fs/core/fspath.h"
#include "zoo/fs/core/source.h"
#include "zoo/fs/core/destination.h"
#include <functional>
#include <cstdint>

namespace zoo {
namespace fs {

/// Move a file from @a source to @a dest using the given @a access.
/// After successful completion, the current_path member of @a source will hold the new path.
/// See destination.h for an explanation on how the destination path is constructed.
ZOO_FS_CORE_API void move_file(iaccess& access, source& source, const destination& dest);

/// Copy a file from @a source using the @a source_access to @a dest using the @a dest_access.
/// The optional @a on_progress callback is called after each block is copied.
/// See destination.h for an explanation on how the destination path is constructed.
ZOO_FS_CORE_API fspath copy_file(iaccess&                                        source_access,
                                 const source&                                   source,
                                 iaccess&                                        dest_access,
                                 const destination&                              dest,
                                 std::function<void(std::uint64_t bytes_copied)> on_progress = nullptr);

} // namespace fs
} // namespace zoo
