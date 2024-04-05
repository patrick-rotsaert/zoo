//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/make_attributes.h"
#include "zoo/fs/core/attributes.h"
#include <gtest/gtest.h>

namespace zoo {
namespace fs {
namespace sftp {

namespace {

std::chrono::system_clock::time_point convert_file_time(uint64_t sec, uint32_t nsec)
{
	return std::chrono::system_clock::from_time_t(static_cast<std::time_t>(sec)) + std::chrono::nanoseconds{ nsec };
}

std::chrono::system_clock::time_point convert_file_time(uint32_t sec)
{
	return std::chrono::system_clock::from_time_t(static_cast<std::time_t>(sec));
}

} // namespace

TEST(MakeAttributesTests, test_attr_permissions)
{
	auto in        = sftp_attributes_struct{};
	in.flags       = SSH_FILEXFER_ATTR_PERMISSIONS;
	in.permissions = S_IFREG | S_IRUSR | S_IWGRP | S_IXOTH;
	const auto a   = make_attributes(&in);
	EXPECT_EQ(a.get_mode(), in.permissions);
}

TEST(MakeAttributesTests, test_file_type)
{
	auto in = sftp_attributes_struct{};
	in.type = SSH_FILEXFER_TYPE_REGULAR;
	EXPECT_EQ(make_attributes(&in).type, attributes::filetype::FILE);
	in.type = SSH_FILEXFER_TYPE_DIRECTORY;
	EXPECT_EQ(make_attributes(&in).type, attributes::filetype::DIR);
	in.type = SSH_FILEXFER_TYPE_SYMLINK;
	EXPECT_EQ(make_attributes(&in).type, attributes::filetype::LINK);
	in.type = SSH_FILEXFER_TYPE_SPECIAL;
	EXPECT_EQ(make_attributes(&in).type, attributes::filetype::SPECIAL);
	in.type = SSH_FILEXFER_TYPE_UNKNOWN;
	EXPECT_EQ(make_attributes(&in).type, attributes::filetype::UNKNOWN);
	in.type = SSH_FILEXFER_TYPE_UNKNOWN + 1;
	EXPECT_EQ(make_attributes(&in).type, attributes::filetype::UNKNOWN);
}

TEST(MakeAttributesTests, test_attr_size)
{
	auto in  = sftp_attributes_struct{};
	in.flags = SSH_FILEXFER_ATTR_SIZE;
	in.size  = 42;
	EXPECT_EQ(make_attributes(&in).size, 42);
	in.flags = 0;
	EXPECT_EQ(make_attributes(&in).size, std::nullopt);
}

TEST(MakeAttributesTests, test_attr_uidgid)
{
	auto in = sftp_attributes_struct{};
	in.uid  = 101;
	in.gid  = 102;
	{
		in.flags     = SSH_FILEXFER_ATTR_UIDGID;
		const auto a = make_attributes(&in);
		EXPECT_EQ(a.uid, 101);
		EXPECT_EQ(a.gid, 102);
	}
	{
		in.flags     = 0;
		const auto a = make_attributes(&in);
		EXPECT_EQ(a.uid, std::nullopt);
		EXPECT_EQ(a.gid, std::nullopt);
	}
}

TEST(MakeAttributesTests, test_attr_mtime)
{
	auto in           = sftp_attributes_struct{};
	in.mtime64        = 12345;
	in.mtime_nseconds = 67890;
	in.mtime          = 45678;
	in.flags          = SSH_FILEXFER_ATTR_MODIFYTIME | SSH_FILEXFER_ATTR_SUBSECOND_TIMES;
	EXPECT_EQ(make_attributes(&in).mtime, convert_file_time(in.mtime64, in.mtime_nseconds));
	in.flags = SSH_FILEXFER_ATTR_ACMODTIME | SSH_FILEXFER_ATTR_SUBSECOND_TIMES;
	EXPECT_EQ(make_attributes(&in).mtime, convert_file_time(in.mtime64, in.mtime_nseconds));
	in.flags = SSH_FILEXFER_ATTR_MODIFYTIME;
	EXPECT_EQ(make_attributes(&in).mtime, convert_file_time(in.mtime));
	in.flags = SSH_FILEXFER_ATTR_ACMODTIME;
	EXPECT_EQ(make_attributes(&in).mtime, convert_file_time(in.mtime));
	in.flags = 0;
	EXPECT_EQ(make_attributes(&in).mtime, std::nullopt);
}

TEST(MakeAttributesTests, test_attr_atime)
{
	auto in           = sftp_attributes_struct{};
	in.atime64        = 12345;
	in.atime_nseconds = 67890;
	in.atime          = 45678;
	in.flags          = SSH_FILEXFER_ATTR_ACCESSTIME | SSH_FILEXFER_ATTR_SUBSECOND_TIMES;
	EXPECT_EQ(make_attributes(&in).atime, convert_file_time(in.atime64, in.atime_nseconds));
	in.flags = SSH_FILEXFER_ATTR_ACMODTIME | SSH_FILEXFER_ATTR_SUBSECOND_TIMES;
	EXPECT_EQ(make_attributes(&in).atime, convert_file_time(in.atime64, in.atime_nseconds));
	in.flags = SSH_FILEXFER_ATTR_ACCESSTIME;
	EXPECT_EQ(make_attributes(&in).atime, convert_file_time(in.atime));
	in.flags = SSH_FILEXFER_ATTR_ACMODTIME;
	EXPECT_EQ(make_attributes(&in).atime, convert_file_time(in.atime));
	in.flags = 0;
	EXPECT_EQ(make_attributes(&in).atime, std::nullopt);
}

TEST(MakeAttributesTests, test_attr_ctime)
{
	auto in                = sftp_attributes_struct{};
	in.createtime          = 12345;
	in.createtime_nseconds = 67890;
	in.flags               = SSH_FILEXFER_ATTR_CREATETIME;
	EXPECT_EQ(make_attributes(&in).ctime, convert_file_time(in.createtime, in.createtime_nseconds));
	in.flags = 0;
	EXPECT_EQ(make_attributes(&in).ctime, std::nullopt);
}

TEST(MakeAttributesTests, test_owner)
{
	auto in      = sftp_attributes_struct{};
	char owner[] = "johndoe";
	in.owner     = owner;
	EXPECT_EQ(make_attributes(&in).owner, owner);
	in.owner = nullptr;
	EXPECT_EQ(make_attributes(&in).owner, std::nullopt);
}

TEST(MakeAttributesTests, test_group)
{
	auto in      = sftp_attributes_struct{};
	char group[] = "johndoe";
	in.group     = group;
	EXPECT_EQ(make_attributes(&in).group, group);
	in.group = nullptr;
	EXPECT_EQ(make_attributes(&in).group, std::nullopt);
}

} // namespace sftp
} // namespace fs
} // namespace zoo
