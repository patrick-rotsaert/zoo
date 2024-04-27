//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/local/local_access.h"
#include "zoo/fs/local/local_file.h"
#include "zoo/fs/local/local_watcher.h"
#include "zoo/fs/local/make_attributes.h"
#include "zoo/fs/local/make_direntry.h"
#include "zoo/fs/core/direntry.h"
#include "zoo/fs/core/exceptions.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/compat.h"
#include <boost/filesystem/operations.hpp>
#include <boost/scoped_array.hpp>
#include <optional>
#include <chrono>

//#define BOOST_STACKTRACE_USE_BACKTRACE
//#include <boost/stacktrace.hpp>

#define THROW_PATH_OP_ERROR(path, opname) ZOO_THROW_EXCEPTION(system_exception{} << error_path{ path } << error_opname{ opname })

namespace zoo {
namespace fs {
namespace local {

access::access(std::shared_ptr<iinterruptor> interruptor)
    : interruptor_{ interruptor }
{
	//zlog(trace,"local access\n{}", boost::stacktrace::stacktrace());
	zlog(trace, "local access");
}

bool access::is_remote() const
{
	return false;
}

std::vector<direntry> access::ls(const fspath& dir)
{
	this->interruptor_->throw_if_interrupted();
	auto result = std::vector<direntry>{};
	for (const auto& entry : boost::filesystem::directory_iterator{ dir })
	{
		this->interruptor_->throw_if_interrupted();
		result.push_back(make_direntry(entry));
	}
	return result;
}

bool access::exists(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();
	zlog(trace, "exists path={}", path);
	return boost::filesystem::exists(path);
}

std::optional<attributes> access::try_stat(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();

	zlog(trace, "try_stat path={}", path);
	auto       ec = boost::system::error_code{};
	const auto st = boost::filesystem::directory_entry{ path }.status(ec);
	if (ec == boost::system::errc::no_such_file_or_directory)
	{
		return std::nullopt;
	}
	else if (ec)
	{
		ZOO_THROW_EXCEPTION(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "status" });
	}

	return make_attributes(path, st);
}

attributes access::stat(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();

	zlog(trace, "stat path={}", path);
	auto       ec = boost::system::error_code{};
	const auto st = boost::filesystem::directory_entry{ path }.status(ec);
	if (ec)
	{
		ZOO_THROW_EXCEPTION(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "status" });
	}

	return make_attributes(path, st);
}

std::optional<attributes> access::try_lstat(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();

	zlog(trace, "lstat path={}", path);
	auto       ec = boost::system::error_code{};
	const auto st = boost::filesystem::directory_entry{ path }.symlink_status(ec);
	if (ec == boost::system::errc::no_such_file_or_directory)
	{
		return std::nullopt;
	}
	else if (ec)
	{
		ZOO_THROW_EXCEPTION(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "symlink_status" });
	}

	return make_attributes(path, st);
}

attributes access::lstat(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();

	zlog(trace, "lstat path={}", path);
	auto       ec = boost::system::error_code{};
	const auto st = boost::filesystem::directory_entry{ path }.symlink_status(ec);
	if (ec)
	{
		ZOO_THROW_EXCEPTION(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "symlink_status" });
	}

	return make_attributes(path, st);
}

void access::remove(const fspath& path)
{
	this->interruptor_->throw_if_interrupted();

	zlog(trace, "remove path={}", path);
	auto ec = boost::system::error_code{};
	boost::filesystem::remove(path, ec);
	if (ec)
	{
		ZOO_THROW_EXCEPTION(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "remove" });
	}
}

void access::mkdir(const fspath& path, bool parents)
{
	this->interruptor_->throw_if_interrupted();

	if (parents)
	{
		zlog(trace, "create_directories path={}", path);
		auto ec = boost::system::error_code{};
		boost::filesystem::create_directories(path, ec);
		if (ec.failed())
		{
			ZOO_THROW_EXCEPTION(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "create_directories" });
		}
	}
	else
	{
		zlog(trace, "create_directory path={}", path);
		auto ec = boost::system::error_code{};
		boost::filesystem::create_directory(path, ec);
		if (ec.failed())
		{
			ZOO_THROW_EXCEPTION(system_exception(std::error_code{ ec }) << error_path{ path } << error_opname{ "create_directory" });
		}
	}
}

void access::rename(const fspath& oldpath, const fspath& newpath)
{
	this->interruptor_->throw_if_interrupted();

	zlog(trace, "rename oldpath={} newpath={}", oldpath, newpath);
	auto ec = boost::system::error_code{};
	boost::filesystem::rename(oldpath, newpath, ec);
	if (ec.failed())
	{
		ZOO_THROW_EXCEPTION(system_exception(std::error_code{ ec })
		                    << error_oldpath{ oldpath } << error_newpath{ newpath } << error_opname{ "rename" });
	}
}

std::unique_ptr<ifile> access::open(const fspath& path, int flags, mode_t mode)
{
	this->interruptor_->throw_if_interrupted();

	zlog(trace, "open path={} flags={:o} mode={:o}", path, flags, mode);
	const auto fd = c_open(path.string().c_str(), flags, mode);
	zlog(trace, "fd {}", fd);
	if (fd == -1)
	{
		THROW_PATH_OP_ERROR(path, "open");
	}
	else
	{
		return std::make_unique<file>(fd, path, this->interruptor_);
	}
}

std::shared_ptr<iwatcher> access::create_watcher(const fspath& dir)
{
	return std::make_shared<watcher>(dir);
}

direntry access::get_direntry(const fspath& path)
{
	return make_direntry(boost::filesystem::directory_entry{ path });
}

} // namespace local
} // namespace fs
} // namespace zoo
