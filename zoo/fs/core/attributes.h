//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"
#include <boost/system/api_config.hpp>
#include <set>
#include <string>
#include <cstddef>
#include <optional>
#include <chrono>

#include <sys/stat.h>

#ifdef BOOST_WINDOWS_API
//#define S_IFDIR  0040000
//#define S_IFCHR  0020000
#define S_IFBLK 0060000
//#define S_IFREG  0100000
#define S_IFIFO 0010000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000
#define S_ISUID 0004000
#define S_ISGID 0002000
#define S_ISVTX 0001000
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

using uid_t = std::uint32_t; // TODO: verify
using gid_t = std::uint32_t; // TODO: verify
#endif

namespace zoo {
namespace fs {

class ZOO_EXPORT attributes final
{
public:
	enum class filetype
	{
		BLOCK,
		CHAR,
		DIR,
		FIFO,
		LINK,
		FILE,
		SOCK,
		SPECIAL, // sftp only
		UNKNOWN,
		BLK = BLOCK,
		CHR = CHAR,
		REG = FILE,
		LNK = LINK
	};

	enum class filemode
	{
		SET_UID,
		SET_GID,
		STICKY
	};
	using filemodes = std::set<filemode>;

	enum class fileperm
	{
		READ,
		WRITE,
		EXEC
	};
	using fileperms = std::set<fileperm>;

	filetype                                             type;
	filemodes                                            mode;
	fileperms                                            uperm, gperm, operm;
	std::optional<uintmax_t>                             size;
	std::optional<uid_t>                                 uid;
	std::optional<gid_t>                                 gid;
	std::optional<std::string>                           owner, group;
	std::optional<std::chrono::system_clock::time_point> atime, mtime, ctime;

	attributes()                             = default;
	attributes(const attributes&)            = default;
	attributes(attributes&& src)             = default;
	attributes& operator=(const attributes&) = default;
	attributes& operator=(attributes&&)      = default;

	bool is_dir() const;
	bool is_reg() const;
	bool is_lnk() const;

	mode_t                     get_mode() const;
	void                       set_mode(mode_t mode);
	std::string                mode_string() const;
	std::optional<std::string> owner_or_uid() const;
	std::optional<std::string> group_or_gid() const;

	bool operator==(const attributes& rhs) const;
};

} // namespace fs
} // namespace zoo
