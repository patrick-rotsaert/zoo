//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/issh_api.h"
#include "zoo/fs/sftp/ptr_types.h"

namespace zoo {
namespace fs {
namespace sftp {

class ssh_server_pubkey final
{
	ssh_key_ptr key_;

public:
	explicit ssh_server_pubkey(issh_api* api, ssh_session session);

	ssh_key_ptr key() const;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
