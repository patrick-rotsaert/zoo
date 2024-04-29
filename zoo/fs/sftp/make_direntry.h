//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/config.h"
#include "zoo/fs/sftp/issh_api.h"
#include "zoo/fs/core/direntry.h"
#include "zoo/fs/core/fspath.h"

namespace zoo {
namespace fs {
namespace sftp {

ZOO_FS_SFTP_LOCAL direntry make_direntry(issh_api* api, const fspath& path, sftp_session sftp, const sftp_attributes a);

}
} // namespace fs
} // namespace zoo
