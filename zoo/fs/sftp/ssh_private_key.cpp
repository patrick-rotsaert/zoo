//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/ssh_private_key.h"
#include "zoo/fs/sftp/sftp_exceptions.h"

namespace zoo {
namespace fs {
namespace sftp {

ssh_private_key::ssh_private_key(issh_api* api, const std::string& b64)
{
	ssh_key    key = nullptr;
	const auto rc  = api->ssh_pki_import_privkey_base64(b64.c_str(), nullptr, nullptr, nullptr, &key);
	if (rc != SSH_OK)
	{
		ZOO_THROW_EXCEPTION(ssh_exception{} << error_opname{ "ssh_pki_import_privkey_base64" });
	}
	const auto keyptr = ssh_key_ptr{ key, [api](auto k) { api->ssh_key_free(k); } };
	if (!api->ssh_key_is_private(key))
	{
		ZOO_THROW_EXCEPTION(ssh_exception{} << error_mesg{ "Not a private key" } << error_opname{ "ssh_key_is_private" });
	}
	this->key_ = keyptr;
}

ssh_key_ptr ssh_private_key::key() const
{
	return this->key_;
}

} // namespace sftp
} // namespace fs
} // namespace zoo
