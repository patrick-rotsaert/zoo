//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/issh_api.h"
#include "zoo/fs/sftp/ssh_private_key.h"
#include "zoo/fs/sftp/ptr_types.h"
#include <string>

namespace zoo {
namespace fs {
namespace sftp {

class ssh_public_key final
{
	ssh_key_ptr key_;

public:
	explicit ssh_public_key(issh_api* api, const ssh_private_key& pkey);

	ssh_key_ptr key() const;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
