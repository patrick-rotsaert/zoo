//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/local/config.h"
#include "zoo/fs/core/direntry.h"
#include "zoo/fs/core/fspath.h"
#include <boost/filesystem/directory.hpp>

namespace zoo {
namespace fs {
namespace local {

ZOO_FS_LOCAL_LOCAL direntry make_direntry(const boost::filesystem::directory_entry& e);

}
} // namespace fs
} // namespace zoo
