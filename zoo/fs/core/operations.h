//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"
#include "zoo/fs/core/iaccess.h"
#include "zoo/fs/core/fspath.h"
#include "zoo/fs/core/source.h"
#include "zoo/fs/core/destination.h"
#include <functional>
#include <cstddef>

namespace zoo {
namespace fs {

// TODO: add documentation
ZOO_EXPORT void move_file(iaccess& access, source& source, const destination& dest);

// TODO: add documentation
ZOO_EXPORT fspath copy_file(iaccess&                                       source_access,
                            const source&                                   source,
                            iaccess&                                       dest_access,
                            const destination&                              dest,
                            std::function<void(std::uint64_t bytes_copied)> on_progress = nullptr);

} // namespace fs
} // namespace zoo
