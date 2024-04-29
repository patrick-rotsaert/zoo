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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace zoo {
namespace lockfile {

namespace detail {

// FIXME: deduplicate
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

// FIXME: deduplicate
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
	int                     fd_;

public:
	explicit impl(const std::filesystem::path& path)

	    : path_{ path }
	    , in_process_lock_{ path }
	    , fd_{ open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600) }
	{
		if (this->fd_ == -1)
		{
			ZOO_THROW_EXCEPTION((std::system_error{ std::error_code{ errno, std::system_category() }, path.string() + ": open" }));
		}

		auto fl     = flock{};
		fl.l_type   = F_WRLCK;
		fl.l_whence = SEEK_SET;
		fl.l_start  = 0;
		fl.l_len    = 0;

		if (fcntl(this->fd_, F_SETLK, &fl) == -1)
		{
			close(this->fd_);
			ZOO_THROW_EXCEPTION(
			    (std::system_error{ std::error_code{ errno, std::system_category() }, path.string() + ": fcntl(F_SETLK)" }));
		}
	}

	~impl() noexcept
	{
		auto ec = std::error_code{};
		std::filesystem::remove(this->path_, ec);
		close(this->fd_);
	}
};

} // namespace lockfile
} // namespace zoo
