//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "sftp_fs_test_fixture.h"
#include "zoo/fs/sftp/sftp_session.h"
#include "zoo/fs/core/noop_interruptor.h"
#include <gtest/gtest.h>

namespace zoo {
namespace fs {
namespace sftp {

class SftpSessionTests : public SftpFsTestFixture
{
protected:
	session make_lazy_session()
	{
		return session{ &this->nice_ssh_api,
			            this->opts,
			            this->nice_ssh_known_hosts,
			            this->nice_ssh_identity_factory,
			            std::make_shared<noop_interruptor>(),
			            true };
	}

	auto run_test(i_ssh_known_hosts::result knownhosts_verify_result,
	              int                       user_auth_method,
	              enum ssh_auth_e           auth_method_result,
	              enum ssh_auth_e           auth_method2_result = SSH_AUTH_SUCCESS)
	{
		auto expect_throw = !this->setup_ssh_calls(knownhosts_verify_result, user_auth_method, auth_method_result, auth_method2_result);
		auto session      = this->make_lazy_session();
		if (expect_throw)
		{
			EXPECT_ANY_THROW(session.ssh());
		}
		else
		{
			EXPECT_NE(session.ssh(), nullptr);
		}
		return session;
	}
};

TEST_F(SftpSessionTests, test_ssh_new_fail)
{
	EXPECT_CALL(this->nice_ssh_api, ssh_new()).Times(1).WillOnce(testing::Return(nullptr));
	EXPECT_CALL(this->nice_ssh_api, ssh_free(testing::_)).Times(0);
	EXPECT_ANY_THROW(this->make_lazy_session().ssh());
}

TEST_F(SftpSessionTests, test_setup_ssh_known_host)
{
	this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_NONE, SSH_AUTH_SUCCESS);
}

TEST_F(SftpSessionTests, test_setup_ssh_unknown_host_allowed)
{
	this->opts.allow_unknown_host_key = true;
	this->run_test(i_ssh_known_hosts::result::UNKNOWN, SSH_AUTH_METHOD_NONE, SSH_AUTH_SUCCESS);
}

TEST_F(SftpSessionTests, test_setup_ssh_unknown_host_not_allowed)
{
	this->opts.allow_unknown_host_key = false;
	this->run_test(i_ssh_known_hosts::result::UNKNOWN, SSH_AUTH_METHOD_NONE, SSH_AUTH_SUCCESS);
}

TEST_F(SftpSessionTests, test_setup_ssh_changed_host_allowed)
{
	this->opts.allow_unknown_host_key = true;
	this->run_test(i_ssh_known_hosts::result::CHANGED, SSH_AUTH_METHOD_NONE, SSH_AUTH_SUCCESS);
}

TEST_F(SftpSessionTests, test_setup_ssh_changed_host_not_allowed)
{
	this->opts.allow_unknown_host_key = false;
	this->run_test(i_ssh_known_hosts::result::CHANGED, SSH_AUTH_METHOD_NONE, SSH_AUTH_SUCCESS);
}

TEST_F(SftpSessionTests, test_setup_ssh_user_auth_none_fail)
{
	this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_NONE, SSH_AUTH_ERROR);
}

TEST_F(SftpSessionTests, test_setup_ssh_user_auth_password_success)
{
	this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_PASSWORD, SSH_AUTH_SUCCESS);
}

TEST_F(SftpSessionTests, test_setup_ssh_user_auth_password_fail)
{
	this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_PASSWORD, SSH_AUTH_ERROR);
}

TEST_F(SftpSessionTests, test_setup_ssh_user_auth_pubkey_success)
{
	this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_PUBLICKEY, SSH_AUTH_SUCCESS, SSH_AUTH_SUCCESS);
}

TEST_F(SftpSessionTests, test_setup_ssh_user_auth_try_pubkey_fail_error)
{
	this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_PUBLICKEY, SSH_AUTH_ERROR);
}

TEST_F(SftpSessionTests, test_setup_ssh_user_auth_try_pubkey_fail_denied)
{
	this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_PUBLICKEY, SSH_AUTH_DENIED);
}

TEST_F(SftpSessionTests, test_setup_ssh_user_auth_pubkey_fail_error)
{
	this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_PUBLICKEY, SSH_AUTH_SUCCESS, SSH_AUTH_ERROR);
}

TEST_F(SftpSessionTests, test_setup_ssh_user_auth_pubkey_fail_denied)
{
	this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_PUBLICKEY, SSH_AUTH_SUCCESS, SSH_AUTH_DENIED);
}

TEST_F(SftpSessionTests, test_setup_sftp)
{
	auto session = this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_NONE, SSH_AUTH_SUCCESS);
	auto sq      = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_new(mock_ssh_api::test_ssh_session))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_session));
	EXPECT_CALL(this->nice_ssh_api, sftp_init(mock_ssh_api::test_sftp_session)).Times(1).WillOnce(testing::Return(SSH_OK));
	EXPECT_CALL(this->nice_ssh_api, sftp_free(mock_ssh_api::test_sftp_session)).Times(1);
	EXPECT_EQ(session.sftp(), mock_ssh_api::test_sftp_session);
}

TEST_F(SftpSessionTests, test_sftp_new_fail)
{
	auto session = this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_NONE, SSH_AUTH_SUCCESS);
	EXPECT_CALL(this->nice_ssh_api, sftp_new(mock_ssh_api::test_ssh_session)).Times(1).WillOnce(testing::Return(nullptr));
	EXPECT_CALL(this->nice_ssh_api, sftp_init(testing::_)).Times(0);
	EXPECT_CALL(this->nice_ssh_api, sftp_free(testing::_)).Times(0);
	EXPECT_ANY_THROW(session.sftp());
}

TEST_F(SftpSessionTests, test_sftp_init_fail)
{
	auto session = this->run_test(i_ssh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_NONE, SSH_AUTH_SUCCESS);
	auto sq      = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_new(mock_ssh_api::test_ssh_session))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_session));
	EXPECT_CALL(this->nice_ssh_api, sftp_init(mock_ssh_api::test_sftp_session)).Times(1).WillOnce(testing::Return(SSH_ERROR));
	EXPECT_CALL(this->nice_ssh_api, sftp_free(mock_ssh_api::test_sftp_session)).Times(1);
	EXPECT_ANY_THROW(session.sftp());
}

} // namespace sftp
} // namespace fs
} // namespace zoo
