//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/sftp_exceptions.h"
#include "zoo/fs/sftp/issh_api.h"

namespace zoo {
namespace fs {
namespace sftp {

namespace {

const char* get_sftp_error_code_name(int err)
{
#define SFTP_ERROR_CODES(_)                                                                                                                \
	_(SSH_FX_OK)                                                                                                                           \
	_(SSH_FX_EOF)                                                                                                                          \
	_(SSH_FX_NO_SUCH_FILE)                                                                                                                 \
	_(SSH_FX_PERMISSION_DENIED)                                                                                                            \
	_(SSH_FX_FAILURE)                                                                                                                      \
	_(SSH_FX_BAD_MESSAGE)                                                                                                                  \
	_(SSH_FX_NO_CONNECTION)                                                                                                                \
	_(SSH_FX_CONNECTION_LOST)                                                                                                              \
	_(SSH_FX_OP_UNSUPPORTED)                                                                                                               \
	_(SSH_FX_INVALID_HANDLE)                                                                                                               \
	_(SSH_FX_NO_SUCH_PATH)                                                                                                                 \
	_(SSH_FX_FILE_ALREADY_EXISTS)                                                                                                          \
	_(SSH_FX_WRITE_PROTECT)                                                                                                                \
	_(SSH_FX_NO_MEDIA)                                                                                                                     \
	// SFT_ERROR_CODES

	switch (err)
	{
#undef expand
#define expand(x)                                                                                                                          \
	case x:                                                                                                                                \
		return #x;
		SFTP_ERROR_CODES(expand) //
	default:
		return "UNKNOWN";
	}
}

} // namespace

ssh_exception::ssh_exception(issh_api* api, ssh_session session)
{
	*this << error_mesg(api->ssh_get_error(session));
	*this << ssh_error_code(api->ssh_get_error_code(session));
}

sftp_exception::sftp_exception(issh_api* api, ssh_session ssh, sftp_session sftp)
    : ssh_exception{ api, ssh }
{
	const auto err = api->sftp_get_error(sftp);
	*this << sftp_error_code(err);
	*this << sftp_error_code_name(get_sftp_error_code_name(err));
}

sftp_exception::sftp_exception(issh_api* api, sftp_session sftp)
    : ssh_exception{}
{
	const auto err = api->sftp_get_error(sftp);
	*this << sftp_error_code(err);
	*this << sftp_error_code_name(get_sftp_error_code_name(err));
}

sftp_exception::sftp_exception(issh_api* api, std::shared_ptr<session> session)
    : ssh_exception{ api, session->ssh() }
{
	const auto err = api->sftp_get_error(session->sftp());
	*this << sftp_error_code(err);
	*this << sftp_error_code_name(get_sftp_error_code_name(err));
}

} // namespace sftp
} // namespace fs
} // namespace zoo
