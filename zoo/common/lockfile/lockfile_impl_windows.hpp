//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/common/lockfile/lockfile.h"
#include "zoo/common/misc/throw_exception.h"
#include "zoo/common/misc/formatters.hpp"

#include <fmt/format.h>

#include <system_error>
#include <mutex>
#include <map>
#include <memory>

#include <windows.h>
#include <fcntl.h>
#include <io.h>

namespace zoo {
namespace lockfile {

namespace detail {

class file_mutex_map_singleton final
{
	std::map<std::filesystem::path, std::unique_ptr<std::mutex>> map_{};
	std::mutex                                                   mutex_{};

	file_mutex_map_singleton()
	{
	}

public:
	static file_mutex_map_singleton& instance()
	{
		static auto inst = file_mutex_map_singleton{};
		return inst;
	}

	std::mutex& get_mutex(const std::filesystem::path& path)
	{
		auto  lock = std::unique_lock{ this->mutex_ };
		auto& mp   = this->map_[path];
		if (!mp.get())
		{
			mp = std::make_unique<std::mutex>();
		}
		return *mp;
	}
};

class in_process_lock final
{
	std::mutex& mutex_;

public:
	explicit in_process_lock(const std::filesystem::path& path)
	    : mutex_{ file_mutex_map_singleton::instance().get_mutex(path) }
	{
		if (!this->mutex_.try_lock())
		{
			ZOO_THROW_EXCEPTION(std::runtime_error(fmt::format("The file {} cannot be locked twice in the same process", path)));
		}
	}

	~in_process_lock()
	{
		this->mutex_.unlock();
	}
};

} // namespace detail

class lockfile::impl final
{
	std::filesystem::path   path_;
	detail::in_process_lock in_process_lock_;
	HANDLE                  fd_;

public:
	explicit impl(const std::filesystem::path& path)
	    : path_{ path }
	    , in_process_lock_{ path }
	    , fd_{ CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL) }
	{
		if (fd_ == INVALID_HANDLE_VALUE)
		{
			ZOO_THROW_EXCEPTION(
			    std::system_error{ static_cast<int>(::GetLastError()), std::system_category(), path.string() + ": CreateFileW" });
		}

		auto overlapped = OVERLAPPED{};
		if (!LockFileEx(this->fd_, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &overlapped))
		{
			CloseHandle(this->fd_);
			ZOO_THROW_EXCEPTION(
			    std::system_error{ static_cast<int>(::GetLastError()), std::system_category(), path.string() + ": LockFileEx" });
		}
	}

	~impl() noexcept
	{
		CloseHandle(this->fd_);
		std::error_code ec;
		std::filesystem::remove(this->path_, ec);
	}
};

} // namespace lockfile
} // namespace zoo
