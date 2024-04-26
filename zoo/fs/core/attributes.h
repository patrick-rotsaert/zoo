//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"
#include "zoo/common/compat.h"
#include <boost/system/api_config.hpp>
#include <set>
#include <string>
#include <cstddef>
#include <optional>
#include <chrono>

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

	attributes()                  = default;
	attributes(const attributes&) = default;
	attributes(attributes&& src)  = default;
	attributes& operator=(const attributes&) = default;
	attributes& operator=(attributes&&) = default;

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
