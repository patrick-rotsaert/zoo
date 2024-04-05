//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/sftp_options.h"
#include "zoo/fs/sftp/issh_api.h"
#include "zoo/fs/sftp/issh_knownhosts.h"
#include "zoo/fs/sftp/issh_identity_factory.h"
#include "zoo/fs/core/iinterruptor.h"
#include "zoo/common/api.h"
#include <memory>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

namespace zoo {
namespace fs {
namespace sftp {

class ZOO_EXPORT session final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit session(issh_api*                              api,
	                 const options&                          opts,
	                 std::shared_ptr<i_ssh_known_hosts>      known_hosts,
	                 std::shared_ptr<issh_identity_factory> ssh_identity_factory,
	                 std::shared_ptr<iinterruptor>          interruptor,
	                 bool                                    lazy);
	~session() noexcept;

	session(const session&) = delete;
	session(session&& src);
	session& operator=(const session&) = delete;
	session& operator=(session&&)      = default;

	ssh_session  ssh();
	sftp_session sftp();
};

} // namespace sftp
} // namespace fs
} // namespace zoo
