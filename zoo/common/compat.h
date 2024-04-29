//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/system/api_config.hpp>

#ifdef BOOST_POSIX_API
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef BOOST_WINDOWS_API
#include <io.h>
#include <fcntl.h>
#include <cstdint>
#endif

#ifdef BOOST_WINDOWS_API
#define O_RDONLY _O_RDONLY
#define O_WRONLY _O_WRONLY
#define O_RDWR _O_RDWR
#define O_APPEND _O_APPEND
#define O_CREAT _O_CREAT
#define O_TRUNC _O_TRUNC
#define O_EXCL _O_EXCL
#define O_TEXT _O_TEXT
#define O_BINARY _O_BINARY
#endif

#ifdef BOOST_POSIX_API
#define O_BINARY 0 // does not exist in POSIX
#endif

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
#endif

#ifdef BOOST_WINDOWS_API
namespace zoo {

using uid_t   = std::uint32_t;
using gid_t   = std::uint32_t;
using mode_t  = int;
using off64_t = std::int64_t;

} // namespace zoo
#endif

#ifdef _MSC_VER
#define c_open(pathname, flags, mode) ::_open(pathname, flags, mode)
#define c_close(fd) ::_close(fd)
#define c_read(fd, buf, count) ::_read(fd, buf, static_cast<unsigned int>(count))
#define c_write(fd, buf, count) ::_write(fd, buf, static_cast<unsigned int>(count))
#define c_lseek64(fd, offset, whence) ::_lseeki64(fd, offset, whence)
#else
#define c_open(pathname, flags, mode) ::open(pathname, flags, mode)
#define c_close(fd) ::close(fd)
#define c_read(fd, buf, count) ::read(fd, buf, count)
#define c_write(fd, buf, count) ::write(fd, buf, count)
#define c_lseek64(fd, offset, whence) ::lseek64(fd, offset, whence)
#endif
