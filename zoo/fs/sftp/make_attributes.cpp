//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/make_attributes.h"
#include "zoo/common/logging/logging.h"

namespace zoo {
namespace fs {
namespace sftp {

namespace {

attributes::filetype convert_file_type(const sftp_attributes in)
{
	switch (in->type)
	{
	case SSH_FILEXFER_TYPE_REGULAR:
		return attributes::filetype::FILE;
	case SSH_FILEXFER_TYPE_DIRECTORY:
		return attributes::filetype::DIR;
	case SSH_FILEXFER_TYPE_SYMLINK:
		return attributes::filetype::LINK;
	case SSH_FILEXFER_TYPE_SPECIAL:
		return attributes::filetype::SPECIAL;
	case SSH_FILEXFER_TYPE_UNKNOWN:
	default:
		return attributes::filetype::UNKNOWN;
	}
}

std::chrono::system_clock::time_point convert_file_time(uint64_t sec, uint32_t nsec)
{
	return std::chrono::system_clock::from_time_t(static_cast<std::time_t>(sec)) +
	       std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::nanoseconds{ nsec });
}

std::chrono::system_clock::time_point convert_file_time(uint32_t sec)
{
	return std::chrono::system_clock::from_time_t(static_cast<std::time_t>(sec));
}

} // namespace

attributes make_attributes(const sftp_attributes in)
{
	ZOO_LOG(trace, "flags {0:x}h {0:b}b", in->flags);

	auto result = attributes{};

	if (in->flags & SSH_FILEXFER_ATTR_PERMISSIONS)
	{
		result.set_mode(in->permissions);
	}
	else
	{
		result.type = convert_file_type(in);
	}

	if (in->flags & SSH_FILEXFER_ATTR_SIZE)
	{
		result.size = in->size;
	}

	if (in->flags & SSH_FILEXFER_ATTR_UIDGID)
	{
		result.uid = in->uid;
		result.gid = in->gid;
	}

	if (in->flags & (SSH_FILEXFER_ATTR_MODIFYTIME | SSH_FILEXFER_ATTR_ACMODTIME))
	{
		result.mtime = in->flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? convert_file_time(in->mtime64, in->mtime_nseconds)
		                                                             : convert_file_time(in->mtime);
	}

	if (in->flags & (SSH_FILEXFER_ATTR_ACCESSTIME | SSH_FILEXFER_ATTR_ACMODTIME))
	{
		result.atime = in->flags & SSH_FILEXFER_ATTR_SUBSECOND_TIMES ? convert_file_time(in->atime64, in->atime_nseconds)
		                                                             : convert_file_time(in->atime);
	}

	if (in->flags & SSH_FILEXFER_ATTR_CREATETIME)
	{
		result.ctime = convert_file_time(in->createtime, in->createtime_nseconds);
	}

	if (in->owner)
	{
		result.owner = in->owner;
	}

	if (in->group)
	{
		result.group = in->group;
	}

	return result;
}

} // namespace sftp
} // namespace fs
} // namespace zoo
