//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/ssh_pubkey_hash.h"
#include "zoo/fs/sftp/sftp_exceptions.h"

namespace zoo {
namespace fs {
namespace sftp {

ssh_pubkey_hash::ssh_pubkey_hash(issh_api* api, ssh_session session, ssh_key key, ssh_publickey_hash_type type)
{
	unsigned char* hash = nullptr;
	auto           hlen = size_t{};
	const auto     rc   = api->ssh_get_publickey_hash(key, type, &hash, &hlen);
	if (rc < 0)
	{
		ZOO_THROW_EXCEPTION(ssh_exception(api, session) << error_opname{ "ssh_get_publickey_hash" });
	}
	assert(hash);
	auto hexa = api->ssh_get_hexa(hash, hlen);
	assert(hexa);
	api->ssh_clean_pubkey_hash(&hash);
	this->hash_.assign(hexa);
	api->ssh_string_free_char(hexa);
}

const std::string ssh_pubkey_hash::hash() const
{
	return this->hash_;
}

} // namespace sftp
} // namespace fs
} // namespace zoo
