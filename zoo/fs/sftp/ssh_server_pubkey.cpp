//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/ssh_server_pubkey.h"
#include "zoo/fs/sftp/sftp_exceptions.h"

namespace zoo {
namespace fs {
namespace sftp {

ssh_server_pubkey::ssh_server_pubkey(issh_api* api, ssh_session session)
{
	ssh_key    key = nullptr;
	const auto rc  = api->ssh_get_server_publickey(session, &key);
	if (rc < 0)
	{
		ZOO_THROW_EXCEPTION(ssh_exception(api, session) << error_opname{ "ssh_get_server_publickey" });
	}
	this->key_ = ssh_key_ptr{ key, [api](auto k) { api->ssh_key_free(k); } };
}

ssh_key_ptr ssh_server_pubkey::key() const
{
	return this->key_;
}

} // namespace sftp
} // namespace fs
} // namespace zoo
