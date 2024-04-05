//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "sftp_fs_test_fixture.h"
#include "zoo/fs/sftp/sftp_exceptions.h"
#include "zoo/fs/core/noop_interruptor.h"
#include <boost/exception/get_error_info.hpp>
#include <gtest/gtest.h>
#include <optional>

namespace zoo {
namespace fs {
namespace sftp {

namespace {

template<class TagType, class ExceptionType>
std::optional<typename TagType::value_type> get_error_info(const ExceptionType& e)
{
	if (const auto x = boost::get_error_info<TagType>(e))
	{
		return *x;
	}
	else
	{
		return std::nullopt;
	}
}

} // namespace

class ExceptionsTests : public SftpFsTestFixture
{
protected:
	std::shared_ptr<session> make_session()
	{
		return std::make_shared<session>(&this->nice_ssh_api,
		                                 this->opts,
		                                 this->nice_ssh_known_hosts,
		                                 this->nice_ssh_identity_factory,
		                                 std::make_shared<noop_interruptor>(),
		                                 true);
	}
};

TEST_F(ExceptionsTests, test_ssh_exception_constructor)
{
	constexpr auto m  = "Some error message";
	constexpr int  ec = 42;
	EXPECT_CALL(this->nice_ssh_api, ssh_get_error(mock_ssh_api::test_ssh_session)).Times(1).WillOnce(testing::Return(m));
	EXPECT_CALL(this->nice_ssh_api, ssh_get_error_code(mock_ssh_api::test_ssh_session)).Times(1).WillOnce(testing::Return(ec));
	const auto e = ssh_exception(&this->nice_ssh_api, mock_ssh_api::test_ssh_session);
	EXPECT_EQ(get_error_info<error_mesg>(e), m);
	EXPECT_EQ(get_error_info<ssh_exception::ssh_error_code>(e), ec);
}

TEST_F(ExceptionsTests, test_sftp_exception_constructor_with_ssh_and_sftp_session)
{
	constexpr auto m       = "Some error message";
	constexpr int  ec_ssh  = 42;
	constexpr int  ec_sftp = SSH_FX_PERMISSION_DENIED;
	EXPECT_CALL(this->nice_ssh_api, ssh_get_error(mock_ssh_api::test_ssh_session)).Times(1).WillOnce(testing::Return(m));
	EXPECT_CALL(this->nice_ssh_api, ssh_get_error_code(mock_ssh_api::test_ssh_session)).Times(1).WillOnce(testing::Return(ec_ssh));
	EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session)).Times(1).WillOnce(testing::Return(ec_sftp));
	const auto e = sftp_exception(&this->nice_ssh_api, mock_ssh_api::test_ssh_session, mock_ssh_api::test_sftp_session);
	EXPECT_EQ(get_error_info<error_mesg>(e), m);
	EXPECT_EQ(get_error_info<ssh_exception::ssh_error_code>(e), ec_ssh);
	EXPECT_EQ(get_error_info<sftp_exception::sftp_error_code>(e), ec_sftp);
	EXPECT_TRUE(get_error_info<sftp_exception::sftp_error_code_name>(e));
	EXPECT_STREQ(get_error_info<sftp_exception::sftp_error_code_name>(e).value(), "SSH_FX_PERMISSION_DENIED");
}

TEST_F(ExceptionsTests, test_sftp_exception_constructor_with_sftp_session)
{
	constexpr int ec = SSH_FX_NO_CONNECTION;
	EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session)).Times(1).WillOnce(testing::Return(ec));
	const auto e = sftp_exception(&this->nice_ssh_api, mock_ssh_api::test_sftp_session);
	EXPECT_FALSE(get_error_info<error_mesg>(e));
	EXPECT_FALSE(get_error_info<ssh_exception::ssh_error_code>(e));
	EXPECT_EQ(get_error_info<sftp_exception::sftp_error_code>(e), ec);
	EXPECT_TRUE(get_error_info<sftp_exception::sftp_error_code_name>(e));
	EXPECT_STREQ(get_error_info<sftp_exception::sftp_error_code_name>(e).value(), "SSH_FX_NO_CONNECTION");
}

TEST_F(ExceptionsTests, test_sftp_exception_constructor_with_session)
{
	this->setup_sftp_calls();
	constexpr auto m       = "Some error message";
	constexpr int  ec_ssh  = 42;
	constexpr int  ec_sftp = SSH_FX_NO_MEDIA;
	EXPECT_CALL(this->nice_ssh_api, ssh_get_error(testing::_)).Times(1).WillOnce(testing::Return(m));
	EXPECT_CALL(this->nice_ssh_api, ssh_get_error_code(mock_ssh_api::test_ssh_session)).Times(1).WillOnce(testing::Return(ec_ssh));
	EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session)).Times(1).WillOnce(testing::Return(ec_sftp));
	const auto e = sftp_exception(&this->nice_ssh_api, this->make_session());
	EXPECT_EQ(get_error_info<error_mesg>(e), m);
	EXPECT_EQ(get_error_info<ssh_exception::ssh_error_code>(e), ec_ssh);
	EXPECT_EQ(get_error_info<sftp_exception::sftp_error_code>(e), ec_sftp);
	EXPECT_TRUE(get_error_info<sftp_exception::sftp_error_code_name>(e));
	EXPECT_STREQ(get_error_info<sftp_exception::sftp_error_code_name>(e).value(), "SSH_FX_NO_MEDIA");
}

} // namespace sftp
} // namespace fs
} // namespace zoo
