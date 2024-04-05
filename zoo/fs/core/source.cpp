//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/core/source.h"

namespace zoo {
namespace fs {

source::source(const fspath& path)
    : orig_path{ path }
    , current_path{ path }
{
}

} // namespace fs
} // namespace zoo
