//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/sftp_session.h"
#include "zoo/fs/core/exceptions.h"
#include "zoo/common/api.h"
#include <string>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

namespace zoo {
namespace fs {
namespace sftp {

class issh_api;

struct ZOO_EXPORT ssh_exception : public exception
{
	using ssh_error_code = boost::error_info<struct ssh_error_code_, int>;
	using exception::exception;
	explicit ssh_exception(issh_api* api, ssh_session session);
};

struct ZOO_EXPORT ssh_host_key_exception : public ssh_exception
{
	using ssh_exception::ssh_exception;

	using ssh_host             = boost::error_info<struct ssh_host_, std::string>;
	using ssh_host_pubkey_hash = boost::error_info<struct ssh_host_pubkey_hash_, std::string>;
};

struct ZOO_EXPORT ssh_host_key_unknown : public ssh_host_key_exception
{
	using ssh_host_key_exception::ssh_host_key_exception;

	using ssh_host_key_exception::ssh_host;
	using ssh_host_key_exception::ssh_host_pubkey_hash;
};

struct ZOO_EXPORT ssh_host_key_changed : public ssh_host_key_exception
{
	using ssh_host_key_exception::ssh_host_key_exception;

	using ssh_host_key_exception::ssh_host;
	using ssh_host_key_exception::ssh_host_pubkey_hash;
};

struct ZOO_EXPORT ssh_auth_exception : public ssh_exception
{
	using ssh_exception::ssh_exception;
};

struct ZOO_EXPORT sftp_exception : public ssh_exception
{
	using sftp_error_code      = boost::error_info<struct sftp_error_code_, int>;
	using sftp_error_code_name = boost::error_info<struct sftp_error_code_name_, const char*>;
	using ssh_exception::ssh_exception;
	explicit sftp_exception(issh_api* api, ssh_session ssh, sftp_session sftp);
	explicit sftp_exception(issh_api* api, sftp_session sftp);
	explicit sftp_exception(issh_api* api, std::shared_ptr<session> session);
};

} // namespace sftp
} // namespace fs
} // namespace zoo
