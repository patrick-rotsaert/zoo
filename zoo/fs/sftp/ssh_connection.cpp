//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/ssh_connection.h"
#include "zoo/fs/sftp/sftp_exceptions.h"

namespace zoo {
namespace fs {
namespace sftp {

ssh_connection::ssh_connection(issh_api* api, ssh_session_ptr session)
    : api_{ api }
    , session_{ session }
{
	if (this->api_->ssh_connect(this->session_.get()) != SSH_OK)
	{
		ZOO_THROW_EXCEPTION(ssh_exception(api, this->session_.get()) << error_opname{ "ssh_connect" });
	}
}

ssh_connection::~ssh_connection() noexcept
{
	this->api_->ssh_disconnect(this->session_.get());
}

} // namespace sftp
} // namespace fs
} // namespace zoo
