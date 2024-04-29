//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/bitcask/file.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/misc/throw_exception.h"
#include "zoo/common/compat.h"

#include <fmt/format.h>

#include <system_error>
#include <cassert>

namespace zoo {
namespace bitcask {

namespace {

int open_file(const std::filesystem::path& path, int flags, mode_t mode)
{
	ZOO_LOG(trace, "open path={} flags={:o} mode={:o}", path, flags, mode);
	const auto fd = c_open(path.string().c_str(), flags, mode);
	ZOO_LOG(trace, "fd {}", fd);
	if (fd == -1)
	{
		ZOO_THROW_EXCEPTION(std::system_error{ std::error_code{ errno, std::system_category() }, path.string() + ": open" });
	}
	else
	{
		return fd;
	}
}

void close_file(int& fd) noexcept
{
	ZOO_LOG(trace, "close fd={}", fd);
	c_close(fd);
	fd = -1;
}

} // namespace

class file::impl final
{
	int                   fd_;
	std::filesystem::path path_;
	mutable locker        locker_;

public:
	explicit impl(int fd, const std::filesystem::path& path)
	    : fd_{ fd }
	    , path_{ path }
	    , locker_{}
	{
	}

	~impl() noexcept
	{
		close_file(this->fd_);
	}

	void reopen(int flags, mode_t mode)
	{
		const auto lock = this->locker_.lock();
		(void)(lock);

		close_file(this->fd_);
		this->fd_ = open_file(this->path_, flags, mode);
	}

	const std::filesystem::path& path() const noexcept
	{
		return this->path_;
	}

	std::size_t read(void* buf, std::size_t count, read_mode mode) const
	{
		return this->locked_read(this->locker_.lock(), buf, count, mode);
	}

	void write(const void* buf, std::size_t count) const
	{
		return this->locked_write(this->locker_.lock(), buf, count);
	}

	off64_t seek(off64_t offset, int whence) const
	{
		return this->locked_seek(this->locker_.lock(), offset, whence);
	}

	off64_t seek(off64_t offset) const
	{
		return this->locked_seek(this->locker_.lock(), offset);
	}

	off64_t position() const
	{
		return this->locked_position(this->locker_.lock());
	}

	off64_t size() const
	{
		return this->locked_size(this->locker_.lock());
	}

	lock_type lock() const
	{
		return this->locker_.lock();
	}

	std::size_t locked_read(const lock_type&, void* buf, std::size_t count, read_mode mode) const
	{
		if (count == 0u)
		{
			return count;
		}

		ZOO_LOG(trace, "read fd={} count={}", this->fd_, count);
		const auto rc = c_read(this->fd_, buf, count);

		if (rc < 0)
		{
			ZOO_THROW_EXCEPTION(std::system_error{ std::error_code{ errno, std::system_category() }, this->path_.string() + ": read" });
		}

		switch (mode)
		{
		case read_mode::any:
			return static_cast<std::size_t>(rc);
		case read_mode::zero_or_count:
			if (rc == 0 || static_cast<std::size_t>(rc) == count)
			{
				return static_cast<std::size_t>(rc);
			}
			break;
		case read_mode::count:
			if (static_cast<std::size_t>(rc) == count)
			{
				return count;
			}
			break;
		}

		ZOO_THROW_EXCEPTION(std::runtime_error{ fmt::format("{}: read: unexpected end of file", this->path_.string()) });
	}

	void locked_write(const lock_type&, const void* buf, std::size_t count) const
	{
		if (count == 0u)
		{
			return;
		}

		ZOO_LOG(trace, "write fd={} count={}", this->fd_, count);
		const auto rc = c_write(this->fd_, buf, count);
		if (rc < 0 || static_cast<std::size_t>(rc) != count)
		{
			ZOO_THROW_EXCEPTION(std::system_error{ std::error_code{ errno, std::system_category() }, this->path_.string() + ": write" });
		}
	}

	off64_t locked_seek(const lock_type&, off64_t offset, int whence) const
	{
		ZOO_LOG(trace, "seek fd={} offset={} whence={}", this->fd_, offset, whence);
		const auto rc = c_lseek64(this->fd_, offset, whence);
		if (rc == static_cast<off64_t>(-1))
		{
			ZOO_THROW_EXCEPTION(std::system_error{ std::error_code{ errno, std::system_category() }, this->path_.string() + ": seek" });
		}
		return rc;
	}

	off64_t locked_seek(const lock_type& lock, off64_t offset) const
	{
		return this->locked_seek(lock, offset, SEEK_SET);
	}

	off64_t locked_position(const lock_type& lock) const
	{
		return this->locked_seek(lock, 0, SEEK_CUR);
	}

	off64_t locked_size(const lock_type& lock) const
	{
		return this->locked_seek(lock, 0, SEEK_END);
	}
};

file::file(int fd, const std::filesystem::path& path)
    : pimpl_{ std::make_unique<impl>(fd, path) }
{
}

file::~file() noexcept
{
}

std::unique_ptr<file> file::open(const std::filesystem::path& path, int flags, mode_t mode)
{
	return std::make_unique<file>(open_file(path, flags, mode), path);
}

void file::reopen(int flags, mode_t mode)
{
	return this->pimpl_->reopen(flags, mode);
}

const std::filesystem::path& file::path() const noexcept
{
	return this->pimpl_->path();
}

std::size_t file::read(void* buf, std::size_t count, read_mode mode) const
{
	return this->pimpl_->read(buf, count, mode);
}

void file::write(const void* buf, std::size_t count) const
{
	return this->pimpl_->write(buf, count);
}

off64_t file::seek(off64_t offset, int whence) const
{
	return this->pimpl_->seek(offset, whence);
}

off64_t file::seek(off64_t offset) const
{
	return this->pimpl_->seek(offset);
}

off64_t file::position() const
{
	return this->pimpl_->position();
}

off64_t file::size() const
{
	return this->pimpl_->size();
}

lock_type file::lock() const
{
	return this->pimpl_->lock();
}

std::size_t file::locked_read(const lock_type& lock, void* buf, std::size_t count, read_mode mode) const
{
	return this->pimpl_->locked_read(lock, buf, count, mode);
}

void file::locked_write(const lock_type& lock, const void* buf, std::size_t count) const
{
	return this->pimpl_->locked_write(lock, buf, count);
}

off64_t file::locked_seek(const lock_type& lock, off64_t offset, int whence) const
{
	return this->pimpl_->locked_seek(lock, offset, whence);
}

off64_t file::locked_seek(const lock_type& lock, off64_t offset) const
{
	return this->pimpl_->locked_seek(lock, offset);
}

off64_t file::locked_position(const lock_type& lock) const
{
	return this->pimpl_->locked_position(lock);
}

off64_t file::locked_size(const lock_type& lock) const
{
	return this->pimpl_->locked_size(lock);
}

} // namespace bitcask
} // namespace zoo
