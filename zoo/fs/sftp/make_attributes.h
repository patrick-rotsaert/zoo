//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/config.h"
#include "zoo/fs/core/attributes.h"
#include <libssh/sftp.h>

namespace zoo {
namespace fs {
namespace sftp {

ZOO_FS_SFTP_LOCAL attributes make_attributes(const sftp_attributes in);

} // namespace sftp
} // namespace fs
} // namespace zoo
