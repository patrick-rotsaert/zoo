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

class ssh_connection final
{
	issh_api*       api_;
	ssh_session_ptr session_;

public:
	explicit ssh_connection(issh_api* api, ssh_session_ptr session);

	~ssh_connection() noexcept;

	ssh_connection(ssh_connection&&)            = default;
	ssh_connection& operator=(ssh_connection&&) = default;

	ssh_connection(const ssh_connection&)            = delete;
	ssh_connection& operator=(const ssh_connection&) = delete;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
