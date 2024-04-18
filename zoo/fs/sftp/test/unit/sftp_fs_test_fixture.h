//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "mock_ssh_api.h"
#include "mock_ssh_identity_factory.h"
#include "mock_ssh_knownhosts.h"
#include "zoo/fs/sftp/sftp_options.h"
#include "zoo/fs/sftp/sftp_session.h"
#include <gtest/gtest.h>
#include <memory>

namespace zoo {
namespace fs {
namespace sftp {

class SftpFsTestFixture : public testing::Test
{
protected:
	nice_mock_ssh_api                               nice_ssh_api{};
	std::shared_ptr<nice_mock_ssh_identity_factory> nice_ssh_identity_factory = std::make_shared<nice_mock_ssh_identity_factory>();
	std::shared_ptr<nice_mock_ssh_known_hosts>      nice_ssh_known_hosts      = std::make_shared<nice_mock_ssh_known_hosts>();
	options                                         opts = options{ .host = "host", .port = 2222, .user = "user", .password = "password" };
	std::shared_ptr<ssh_identity>                   identity_ = std::make_shared<ssh_identity>();

	SftpFsTestFixture();

	std::vector<std::shared_ptr<ssh_identity>> make_ssh_idents();

	bool setup_ssh_calls(issh_known_hosts::result knownhosts_verify_result,
	                     int                      user_auth_method,
	                     enum ssh_auth_e          auth_method_result,
	                     enum ssh_auth_e          auth_method2_result = SSH_AUTH_SUCCESS);
	void setup_sftp_calls();
};

} // namespace sftp
} // namespace fs
} // namespace zoo
