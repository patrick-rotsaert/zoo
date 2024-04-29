//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/local/config.h"
#include "zoo/fs/core/attributes.h"
#include "zoo/fs/core/fspath.h"
#include <boost/filesystem/file_status.hpp>

namespace zoo {
namespace fs {
namespace local {

ZOO_FS_LOCAL_LOCAL attributes::filetype make_filetype(boost::filesystem::file_type type);
ZOO_FS_LOCAL_LOCAL attributes           make_attributes(const fspath& path, const boost::filesystem::file_status& st);

} // namespace local
} // namespace fs
} // namespace zoo
