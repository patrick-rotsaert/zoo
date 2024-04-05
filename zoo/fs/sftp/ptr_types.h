//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/issh_api.h"
#include <memory>
#include <type_traits>

namespace zoo {
namespace fs {
namespace sftp {

using ssh_key_ptr      = std::shared_ptr<std::remove_pointer<ssh_key>::type>;
using ssh_session_ptr  = std::shared_ptr<std::remove_pointer<ssh_session>::type>;
using sftp_session_ptr = std::shared_ptr<std::remove_pointer<sftp_session>::type>;

} // namespace sftp
} // namespace fs
} // namespace zoo
