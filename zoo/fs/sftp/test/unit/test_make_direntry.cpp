//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "sftp_fs_test_fixture.h"
#include "zoo/fs/sftp/make_direntry.h"
#include "zoo/fs/sftp/make_attributes.h"
#include "zoo/fs/core/attributes.h"
#include <gtest/gtest.h>

namespace zoo {
namespace fs {
namespace sftp {

class MakeDirentryTests : public SftpFsTestFixture
{
};

TEST_F(MakeDirentryTests, test_non_symlink)
{
	const auto p  = fspath{ "/some/file" };
	auto       a  = sftp_attributes_struct{};
	a.flags       = SSH_FILEXFER_ATTR_PERMISSIONS;
	a.permissions = S_IFREG;
	a.name        = const_cast<char*>("file");
	const auto e  = make_direntry(&this->nice_ssh_api, p, mock_ssh_api::test_sftp_session, &a);
	EXPECT_EQ(e.name, a.name);
	EXPECT_EQ(e.attr, make_attributes(&a));
	EXPECT_EQ(e.symlink_target, std::nullopt);
}

TEST_F(MakeDirentryTests, test_symlink)
{
	const auto p        = fspath{ "/some/link" };
	char       target[] = "target";
	EXPECT_CALL(this->nice_ssh_api, sftp_readlink(mock_ssh_api::test_sftp_session, testing::StrEq(p.string())))
	    .Times(1)
	    .WillOnce(testing::Return(strdup(target)));
	auto a        = sftp_attributes_struct{};
	a.flags       = SSH_FILEXFER_ATTR_PERMISSIONS;
	a.permissions = S_IFLNK;
	a.name        = const_cast<char*>("link");
	const auto e  = make_direntry(&this->nice_ssh_api, p, mock_ssh_api::test_sftp_session, &a);
	EXPECT_EQ(e.name, a.name);
	EXPECT_EQ(e.attr, make_attributes(&a));
	EXPECT_EQ(e.symlink_target, target);
}

TEST_F(MakeDirentryTests, test_symlink_readlink_fail)
{
	const auto p = fspath{ "/some/link" };
	EXPECT_CALL(this->nice_ssh_api, sftp_readlink(mock_ssh_api::test_sftp_session, testing::StrEq(p.string())))
	    .Times(1)
	    .WillOnce(testing::ReturnNull());
	auto a        = sftp_attributes_struct{};
	a.flags       = SSH_FILEXFER_ATTR_PERMISSIONS;
	a.permissions = S_IFLNK;
	a.name        = const_cast<char*>("link");
	EXPECT_ANY_THROW(make_direntry(&this->nice_ssh_api, p, mock_ssh_api::test_sftp_session, &a));
}

} // namespace sftp
} // namespace fs
} // namespace zoo
