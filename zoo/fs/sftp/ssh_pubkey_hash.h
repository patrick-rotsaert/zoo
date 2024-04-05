//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/issh_api.h"
#include <string>

namespace zoo {
namespace fs {
namespace sftp {

class ssh_pubkey_hash final
{
	std::string hash_;

public:
	explicit ssh_pubkey_hash(issh_api* api, ssh_session session, ssh_key key, ssh_publickey_hash_type type);

	const std::string hash() const;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
