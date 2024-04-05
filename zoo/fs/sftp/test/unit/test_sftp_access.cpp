//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "sftp_fs_test_fixture.h"
#include "zoo/fs/sftp/sftp_access.h"
#include "zoo/fs/core/noop_interruptor.h"
#include "zoo/fs/core/direntry.h"
#include "zoo/fs/core/ifile.h"
#include <gtest/gtest.h>

namespace zoo {
namespace fs {
namespace sftp {

namespace {
constexpr auto p = "/some/path";
}

class SftpAccessTests : public SftpFsTestFixture
{
protected:
	SftpAccessTests()
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

TEST_F(SftpAccessTests, test_is_remote)
{
	EXPECT_TRUE(this->make_access().is_remote());
}

TEST_F(SftpAccessTests, test_ls_on_empty_dir)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_opendir(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_dir));
	EXPECT_CALL(this->nice_ssh_api, sftp_readdir(mock_ssh_api::test_sftp_session, mock_ssh_api::test_sftp_dir))
	    .Times(1)
	    .WillOnce(testing::ReturnNull());
	EXPECT_CALL(this->nice_ssh_api, sftp_dir_eof(mock_ssh_api::test_sftp_dir)).Times(1).WillOnce(testing::Return(1));
	EXPECT_CALL(this->nice_ssh_api, sftp_closedir(mock_ssh_api::test_sftp_dir)).Times(1);
	EXPECT_EQ(this->make_access().ls(p).size(), 0);
}

TEST_F(SftpAccessTests, test_sftp_opendir_fail)
{
	EXPECT_CALL(this->nice_ssh_api, sftp_opendir(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::ReturnNull());
	EXPECT_ANY_THROW(this->make_access().ls(p));
}

TEST_F(SftpAccessTests, test_sftp_readdir_fail)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_opendir(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_dir));
	EXPECT_CALL(this->nice_ssh_api, sftp_readdir(mock_ssh_api::test_sftp_session, mock_ssh_api::test_sftp_dir))
	    .Times(1)
	    .WillOnce(testing::ReturnNull());
	EXPECT_CALL(this->nice_ssh_api, sftp_dir_eof(mock_ssh_api::test_sftp_dir)).Times(1).WillOnce(testing::Return(0));
	EXPECT_CALL(this->nice_ssh_api, sftp_closedir(mock_ssh_api::test_sftp_dir)).Times(1);
	EXPECT_ANY_THROW(this->make_access().ls(p));
}

TEST_F(SftpAccessTests, test_ls_on_dir_with_1_file)
{
	EXPECT_CALL(this->nice_ssh_api, sftp_opendir(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_dir));
	EXPECT_CALL(this->nice_ssh_api, sftp_readdir(mock_ssh_api::test_sftp_session, mock_ssh_api::test_sftp_dir))
	    .Times(2)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_attributes))
	    .WillRepeatedly(testing::ReturnNull());
	EXPECT_CALL(this->nice_ssh_api, sftp_attributes_free(mock_ssh_api::test_sftp_attributes)).Times(1);
	EXPECT_CALL(this->nice_ssh_api, sftp_dir_eof(mock_ssh_api::test_sftp_dir)).Times(1).WillOnce(testing::Return(1));
	EXPECT_CALL(this->nice_ssh_api, sftp_closedir(mock_ssh_api::test_sftp_dir)).Times(1);
	const auto ls = this->make_access().ls(p);
	EXPECT_EQ(ls.size(), 1);
}

TEST_F(SftpAccessTests, test_exists)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_stat(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_attributes));
	EXPECT_CALL(this->nice_ssh_api, sftp_attributes_free(mock_ssh_api::test_sftp_attributes)).Times(1);
	EXPECT_TRUE(this->make_access().exists(p));
}

TEST_F(SftpAccessTests, test_exists_not_exist)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_stat(mock_ssh_api::test_sftp_session, testing::StrEq(p))).Times(1).WillOnce(testing::ReturnNull());
	EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session))
	    .Times(1)
	    .WillOnce(testing::Return(SSH_FX_NO_SUCH_FILE));
	EXPECT_FALSE(this->make_access().exists(p));
}

TEST_F(SftpAccessTests, test_exists_fail)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_stat(mock_ssh_api::test_sftp_session, testing::StrEq(p))).Times(1).WillOnce(testing::ReturnNull());
	EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session)).Times(1).WillOnce(testing::Return(SSH_FX_FAILURE));
	EXPECT_ANY_THROW(this->make_access().exists(p));
}

TEST_F(SftpAccessTests, test_try_stat)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_stat(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_attributes));
	EXPECT_CALL(this->nice_ssh_api, sftp_attributes_free(mock_ssh_api::test_sftp_attributes)).Times(1);
	EXPECT_TRUE(this->make_access().try_stat(p).has_value());
}

TEST_F(SftpAccessTests, test_try_stat_not_exist)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_stat(mock_ssh_api::test_sftp_session, testing::StrEq(p))).Times(1).WillOnce(testing::ReturnNull());
	EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session))
	    .Times(1)
	    .WillOnce(testing::Return(SSH_FX_NO_SUCH_FILE));
	EXPECT_FALSE(this->make_access().try_stat(p).has_value());
}

TEST_F(SftpAccessTests, test_try_stat_fail)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_stat(mock_ssh_api::test_sftp_session, testing::StrEq(p))).Times(1).WillOnce(testing::ReturnNull());
	EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session)).Times(1).WillOnce(testing::Return(SSH_FX_FAILURE));
	EXPECT_ANY_THROW(this->make_access().try_stat(p));
}

TEST_F(SftpAccessTests, test_stat)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_stat(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_attributes));
	EXPECT_CALL(this->nice_ssh_api, sftp_attributes_free(mock_ssh_api::test_sftp_attributes)).Times(1);
	EXPECT_NO_THROW(this->make_access().stat(p));
}

TEST_F(SftpAccessTests, test_stat_not_exist)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_stat(mock_ssh_api::test_sftp_session, testing::StrEq(p))).Times(1).WillOnce(testing::ReturnNull());
	EXPECT_ANY_THROW(this->make_access().stat(p));
}

TEST_F(SftpAccessTests, test_try_lstat)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_lstat(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_attributes));
	EXPECT_CALL(this->nice_ssh_api, sftp_attributes_free(mock_ssh_api::test_sftp_attributes)).Times(1);
	EXPECT_TRUE(this->make_access().try_lstat(p).has_value());
}

TEST_F(SftpAccessTests, test_try_lstat_not_exist)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_lstat(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::ReturnNull());
	EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session))
	    .Times(1)
	    .WillOnce(testing::Return(SSH_FX_NO_SUCH_FILE));
	EXPECT_FALSE(this->make_access().try_lstat(p).has_value());
}

TEST_F(SftpAccessTests, test_try_lstat_fail)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_lstat(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::ReturnNull());
	EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session)).Times(1).WillOnce(testing::Return(SSH_FX_FAILURE));
	EXPECT_ANY_THROW(this->make_access().try_lstat(p));
}

TEST_F(SftpAccessTests, test_lstat)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_lstat(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_attributes));
	EXPECT_CALL(this->nice_ssh_api, sftp_attributes_free(mock_ssh_api::test_sftp_attributes)).Times(1);
	EXPECT_NO_THROW(this->make_access().lstat(p));
}

TEST_F(SftpAccessTests, test_lstat_not_exist)
{
	auto sq = testing::InSequence{};
	EXPECT_CALL(this->nice_ssh_api, sftp_lstat(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
	    .Times(1)
	    .WillOnce(testing::ReturnNull());
	EXPECT_ANY_THROW(this->make_access().lstat(p));
}

TEST_F(SftpAccessTests, test_remove)
{
	EXPECT_CALL(this->nice_ssh_api, sftp_unlink(mock_ssh_api::test_sftp_session, testing::StrEq(p))).Times(1).WillOnce(testing::Return(0));
	EXPECT_NO_THROW(this->make_access().remove(p));
}

TEST_F(SftpAccessTests, test_remove_fail)
{
	EXPECT_CALL(this->nice_ssh_api, sftp_unlink(mock_ssh_api::test_sftp_session, testing::StrEq(p))).Times(1).WillOnce(testing::Return(-1));
	EXPECT_ANY_THROW(this->make_access().remove(p));
}

TEST_F(SftpAccessTests, test_mkdir_with_parents)
{
	auto a = this->make_access();
	{
		constexpr auto p  = "/some/path";
		auto           sq = testing::InSequence{};
		EXPECT_CALL(this->nice_ssh_api, sftp_mkdir(mock_ssh_api::test_sftp_session, testing::StrEq(p), 0777))
		    .Times(1)
		    .WillOnce(testing::Return(0));
		EXPECT_NO_THROW(a.mkdir(p, true));
	}
	{
		const auto p  = fspath{ "/some/path" };
		auto       sq = testing::InSequence{};
		EXPECT_CALL(this->nice_ssh_api, sftp_mkdir(mock_ssh_api::test_sftp_session, testing::Eq(p), 0777))
		    .Times(1)
		    .WillOnce(testing::Return(-1));
		EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session))
		    .Times(1)
		    .WillOnce(testing::Return(SSH_FX_NO_SUCH_FILE));
		EXPECT_CALL(this->nice_ssh_api, sftp_mkdir(mock_ssh_api::test_sftp_session, testing::Eq(p.parent_path()), 0777))
		    .Times(1)
		    .WillOnce(testing::Return(0));
		EXPECT_CALL(this->nice_ssh_api, sftp_mkdir(mock_ssh_api::test_sftp_session, testing::Eq(p), 0777))
		    .Times(1)
		    .WillOnce(testing::Return(0));
		EXPECT_NO_THROW(a.mkdir(p, true));
	}
}

TEST_F(SftpAccessTests, test_mkdir_without_parents)
{
	auto a = this->make_access();
	{
		constexpr auto p  = "/some/path";
		auto           sq = testing::InSequence{};
		EXPECT_CALL(this->nice_ssh_api, sftp_mkdir(mock_ssh_api::test_sftp_session, testing::StrEq(p), 0777))
		    .Times(1)
		    .WillOnce(testing::Return(0));
		EXPECT_NO_THROW(a.mkdir(p, false));
	}
	{
		const auto p  = fspath{ "/some/path" };
		auto       sq = testing::InSequence{};
		EXPECT_CALL(this->nice_ssh_api, sftp_mkdir(mock_ssh_api::test_sftp_session, testing::Eq(p), 0777))
		    .Times(1)
		    .WillOnce(testing::Return(-1));
		EXPECT_CALL(this->nice_ssh_api, sftp_get_error(mock_ssh_api::test_sftp_session)).Times(0);
		EXPECT_ANY_THROW(a.mkdir(p, false));
	}
}

TEST_F(SftpAccessTests, test_rename)
{
	auto       a       = this->make_access();
	const auto oldpath = "/some/old";
	const auto newpath = "/some/new";
	{
		EXPECT_CALL(this->nice_ssh_api, sftp_rename(mock_ssh_api::test_sftp_session, testing::StrEq(oldpath), testing::StrEq(newpath)))
		    .Times(1)
		    .WillOnce(testing::Return(0));
		EXPECT_NO_THROW(a.rename(oldpath, newpath));
	}
	{
		EXPECT_CALL(this->nice_ssh_api, sftp_rename(mock_ssh_api::test_sftp_session, testing::StrEq(oldpath), testing::StrEq(newpath)))
		    .Times(1)
		    .WillOnce(testing::Return(-1));
		EXPECT_ANY_THROW(a.rename(oldpath, newpath));
	}
}

TEST_F(SftpAccessTests, test_open)
{
	auto           a = this->make_access();
	constexpr auto p = "/some/file";
	{
		EXPECT_CALL(this->nice_ssh_api, sftp_open(mock_ssh_api::test_sftp_session, testing::StrEq(p), O_RDWR, 0664))
		    .Times(1)
		    .WillOnce(testing::Return(mock_ssh_api::test_sftp_file));
		EXPECT_NO_THROW(a.open(p, O_RDWR, 0664));
	}
	{
		EXPECT_CALL(this->nice_ssh_api, sftp_open(mock_ssh_api::test_sftp_session, testing::StrEq(p), O_RDWR, 0664))
		    .Times(1)
		    .WillOnce(testing::ReturnNull());
		EXPECT_ANY_THROW(a.open(p, O_RDWR, 0664));
	}
}

TEST_F(SftpAccessTests, test_create_watcher)
{
	auto           a = this->make_access();
	constexpr auto p = "/some/dir";
	{
		auto sq = testing::InSequence{};
		EXPECT_CALL(this->nice_ssh_api, sftp_opendir(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
		    .Times(1)
		    .WillOnce(testing::Return(mock_ssh_api::test_sftp_dir));
		EXPECT_CALL(this->nice_ssh_api, sftp_readdir(mock_ssh_api::test_sftp_session, mock_ssh_api::test_sftp_dir))
		    .Times(1)
		    .WillOnce(testing::ReturnNull());
		EXPECT_CALL(this->nice_ssh_api, sftp_dir_eof(mock_ssh_api::test_sftp_dir)).Times(1).WillOnce(testing::Return(1));
		EXPECT_CALL(this->nice_ssh_api, sftp_closedir(mock_ssh_api::test_sftp_dir)).Times(1);
		EXPECT_NO_THROW(a.create_watcher(p, 0));
	}
	{
		auto sq = testing::InSequence{};
		EXPECT_CALL(this->nice_ssh_api, sftp_opendir(mock_ssh_api::test_sftp_session, testing::StrEq(p)))
		    .Times(1)
		    .WillOnce(testing::ReturnNull());
		EXPECT_ANY_THROW(a.create_watcher(p, 0));
	}
}

} // namespace sftp
} // namespace fs
} // namespace zoo
