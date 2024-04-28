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

namespace zoo {
namespace fs {

ZOO_FS_CORE_LOCAL fspath make_dest_path(iaccess& source_access, const source& source, iaccess& dest_access, const destination& dest);

} // namespace fs
} // namespace zoo
