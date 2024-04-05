//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "sftp_fs_test_fixture.h"
#include "zoo/fs/sftp/sftp_access.h"
#include "zoo/fs/sftp/sftp_file.h"
#include "zoo/fs/core/noop_interruptor.h"
#include <gtest/gtest.h>

namespace zoo {
namespace fs {
namespace sftp {

//class SftpFileTests : public SftpFsTestFixture
//{
//protected:
//	SftpFileTests()
//	{
//		this->setup_sftp_calls();
//	}

//	std::shared_ptr<session> make_session()
//	{
//		return std::make_shared<session>(&this->nice_ssh_api,
//		                                 this->opts,
//		                                 this->nice_ssh_known_hosts,
//		                                 this->nice_ssh_identity_factory,
//		                                 std::make_shared<noop_interruptor>(),
//		                                 true);
//	}
//};

class SftpFileTests : public SftpFsTestFixture
{
protected:
	SftpFileTests()
	{
		this->setup_sftp_calls();
	}

	access make_access()
	{
		return access{ this->nice_ssh_api,
			           this->opts,
			           this->nice_ssh_known_hosts,
			           this->nice_ssh_identity_factory,
			           std::make_shared<noop_interruptor>(),
			           false };
	}
};

TEST_F(SftpFileTests, test_read)
{
	auto           a = this->make_access();
	constexpr auto p = "/some/file";
	EXPECT_CALL(this->nice_ssh_api, sftp_open(mock_ssh_api::test_sftp_session, testing::StrEq(p), O_RDONLY, 0664))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_file));
	auto  f   = a.open(p, O_RDONLY, 0664);
	auto  sq  = testing::InSequence{};
	void* buf = nullptr;
	EXPECT_CALL(this->nice_ssh_api, sftp_read(mock_ssh_api::test_sftp_file, buf, 42)).Times(1).WillOnce(testing::Return(12));
	EXPECT_CALL(this->nice_ssh_api, sftp_close(mock_ssh_api::test_sftp_file)).Times(1);
	EXPECT_EQ(f->read(buf, 42), 12);
}

TEST_F(SftpFileTests, test_read_fail)
{
	auto           a = this->make_access();
	constexpr auto p = "/some/file";
	EXPECT_CALL(this->nice_ssh_api, sftp_open(mock_ssh_api::test_sftp_session, testing::StrEq(p), O_RDONLY, 0664))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_file));
	auto  f   = a.open(p, O_RDONLY, 0664);
	auto  sq  = testing::InSequence{};
	void* buf = nullptr;
	EXPECT_CALL(this->nice_ssh_api, sftp_read(mock_ssh_api::test_sftp_file, buf, 42)).Times(1).WillOnce(testing::Return(-1));
	EXPECT_CALL(this->nice_ssh_api, sftp_close(mock_ssh_api::test_sftp_file)).Times(1);
	EXPECT_ANY_THROW(f->read(buf, 42));
}

} // namespace sftp
} // namespace fs
} // namespace zoo
