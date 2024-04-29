//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/fspath.h"
#include "zoo/fs/core/config.h"
#include "zoo/common/misc/throw_exception.h"

#include <boost/exception/exception.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/errinfo_nested_exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/filesystem/path.hpp>
#include <exception>
#include <string>
#include <system_error>

namespace zoo {
namespace fs {

// clang-format off
#undef FS_ERROR_INFO_TAGS
#define FS_ERROR_INFO_TAGS(_e_) \
	_e_(error_uuid, boost::uuids::uuid) \
	_e_(error_code, std::error_code) \
	_e_(error_mesg, std::string) \
	_e_(error_path, fspath) \
	_e_(error_oldpath, fspath) \
	_e_(error_newpath, fspath) \
	_e_(error_opname, std::string) \
	// FS_ERROR_INFO_TAGS
// clang-format on
#undef EXPAND
#define EXPAND(name, type) using name = boost::error_info<struct name##_, type>;
FS_ERROR_INFO_TAGS(EXPAND)

class ZOO_FS_CORE_API exception : public std::exception, public boost::exception
{
public:
	using mesg = error_mesg;

	exception() noexcept;
	explicit exception(std::error_code ec) noexcept;
	explicit exception(const std::string& message) noexcept;
	explicit exception(std::error_code ec, const std::string& message) noexcept;

	const char* what() const noexcept override;
};

struct ZOO_FS_CORE_API should_not_happen_exception : public exception
{
};

struct ZOO_FS_CORE_API invalid_argument_exception : public exception
{
};

class ZOO_FS_CORE_API system_exception : public exception
{
public:
	system_exception();
	explicit system_exception(std::error_code ec);
	explicit system_exception(const std::string& message);
	explicit system_exception(std::error_code ec, const std::string& message);

	static std::error_code getLastErrorCode() noexcept;
};

} // namespace fs
} // namespace zoo
