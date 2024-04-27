//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/local/local_file.h"
#include "zoo/fs/core/exceptions.h"
#include "zoo/common/logging/logging.h"

#include <boost/system/api_config.hpp>

#ifdef BOOST_WINDOWS_API
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef BOOST_WINDOWS_API
#define c_close(fd) ::_close(fd)
#define c_read(fd, buf, count) ::_read(fd, buf, static_cast<unsigned int>(count))
#define c_write(fd, buf, count) ::_write(fd, buf, static_cast<unsigned int>(count))
#else
#define c_close(fd) ::close(fd)
#define c_read(fd, buf, count) ::read(fd, buf, count)
#define c_write(fd, buf, count) ::write(fd, buf, count)
#endif

namespace zoo {
namespace fs {
namespace local {

file::file(int fd, const fspath& path, std::shared_ptr<iinterruptor> interruptor)
    : fd_{ fd }
    , path_{ path }
    , interruptor_{ interruptor }
{
}

file::~file() noexcept
{
	ZOO_LOG(trace, "close fd={}", this->fd_);
	c_close(this->fd_);
}

std::size_t file::read(void* buf, std::size_t count)
{
	this->interruptor_->throw_if_interrupted();
	ZOO_LOG(trace, "read fd={} count={}", this->fd_, count);
	auto rc = c_read(this->fd_, buf, count);
	if (rc < 0)
	{
		ZOO_THROW_EXCEPTION(system_exception{} << error_opname{ "read" } << error_path{ this->path_ });
	}
	return static_cast<std::size_t>(rc);
}

std::size_t file::write(const void* buf, std::size_t count)
{
	this->interruptor_->throw_if_interrupted();
	ZOO_LOG(trace, "write fd={} count={}", this->fd_, count);
	auto rc = c_write(this->fd_, buf, count);
	if (rc < 0)
	{
		ZOO_THROW_EXCEPTION(system_exception{} << error_opname{ "write" } << error_path{ this->path_ });
	}
	return static_cast<std::size_t>(rc);
}

} // namespace local
} // namespace fs
} // namespace zoo
