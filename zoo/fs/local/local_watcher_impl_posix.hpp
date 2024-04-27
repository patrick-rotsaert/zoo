//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/local/local_watcher.h"
#include "zoo/fs/local/local_access.h"
#include "zoo/fs/core/exceptions.h"
#include "zoo/fs/core/iinterruptor.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

#include <boost/algorithm/string/join.hpp>
#include <boost/thread/interruption.hpp>
#include <boost/filesystem/operations.hpp>

#include <fmt/format.h>
#include <cassert>
#include <optional>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

namespace zoo {
namespace fs {
namespace local {

namespace {

#ifdef RFU
bool have_permission(const attributes& attr, attributes::fileperm perm)
{
	assert(attr.uid && attr.gid);
	if (attr.uid.get() == getuid())
	{
		if (attr.uperm.count(perm))
		{
			return true;
		}
	}
	if (attr.gid.get() == getgid())
	{
		if (attr.gperm.count(perm))
		{
			return true;
		}
	}
	return attr.operm.count(perm) > 0;
}

bool have_permission(const direntry& entry, attributes::fileperm perm)
{
	return have_permission(entry.attr, perm) && (!entry.link || have_permission(entry.link.get(), perm));
}

bool have_read_permission(const direntry& entry)
{
	return have_permission(entry, attributes::fileperm::READ);
}

bool have_write_permission(const direntry& entry)
{
	return have_permission(entry, attributes::fileperm::WRITE);
}

bool have_read_and_write_permission(const direntry& entry)
{
	return have_read_permission(entry) && have_write_permission(entry);
}
#endif // RFU

std::string describe_flags(uint32_t mask)
{
	auto flags = std::vector<std::string>{};

	if (mask & IN_ACCESS)
		flags.push_back("IN_ACCESS");
	if (mask & IN_MODIFY)
		flags.push_back("IN_MODIFY");
	if (mask & IN_ATTRIB)
		flags.push_back("IN_ATTRIB");
	if (mask & IN_CLOSE_WRITE)
		flags.push_back("IN_CLOSE_WRITE");
	if (mask & IN_CLOSE_NOWRITE)
		flags.push_back("IN_CLOSE_NOWRITE");
	if (mask & IN_OPEN)
		flags.push_back("IN_OPEN");
	if (mask & IN_MOVED_FROM)
		flags.push_back("IN_MOVED_FROM");
	if (mask & IN_MOVED_TO)
		flags.push_back("IN_MOVED_TO");
	if (mask & IN_CREATE)
		flags.push_back("IN_CREATE");
	if (mask & IN_DELETE)
		flags.push_back("IN_DELETE");
	if (mask & IN_DELETE_SELF)
		flags.push_back("IN_DELETE_SELF");
	if (mask & IN_MOVE_SELF)
		flags.push_back("IN_MOVE_SELF");
	if (mask & IN_UNMOUNT)
		flags.push_back("IN_UNMOUNT");
	if (mask & IN_Q_OVERFLOW)
		flags.push_back("IN_Q_OVERFLOW");
	if (mask & IN_IGNORED)
		flags.push_back("IN_IGNORED");

	return boost::algorithm::join(flags, "|");
}

} // namespace

// NOTE
// This implementation of an inotify "client" only reacts to the events IN_CLOSE_WRITE and IN_MOVED_TO.
// So only files that are closed for write or files that are moved/renamed in(to) this directory are "noticed".
// The event IN_ATTRIB could also be useful to detect attribute changes, e.g. when a file is "noticed" for which
// we have no r/w access, we could ignore the file and wait for the IN_ATTRIB to test r/w access again, so the
// file producer can first create the file and then change the permissions. But this is harder than it seems.
// An IN_ATTRIB event can also happen before the file is closed, so then we'd need to keep track of all files
// that are currently open. It is certainly doable, but it would still be possible to trick this watcher into errors,
// e.g. by creating a file in another directory, then moving it into the monitored directory and then closing it.
// This will definitely cause errors to happen, but we should'n try to make this application fool proof.
// This application is just a tool and we need to keep its functionality as simple as possible. A user of this
// application should take measures to ensure that this application has proper access to the monitored directory.
// EDIT 2021-05-25
// When the monitored directory receives files via the openssh-sftp server, the rename behavior depends on the
// used sftp client.
// The openssh-sftp server has 2 methods for file renames:
// 1) posix-rename:
//    - calls rename(old, new)
//    - this is reported by inotify as 2 events:
//       a) IN_MOVED_FROM (old)
//       b) IN_MOVED_TO (new)
//    - this is easy, we just need the IN_MOVED_TO
// 2) rename:
//    - calls link(old, new) and unlink(old)
//    - this is reported by inotify as 2 events:
//       a) IN_CREATE (new)
//       b) IN_DELETE (old)
//    - we need to distinguish this from a file being created and opened, which is reported by inotify as:
//       a) IN_CREATE (somefile)
//       b) IN_OPEN (somefile)
//    The a+b events for both cases always seem to come in the same message and consecutively.
//    We'll rely on this behavior.

class watcher::impl final
{
	fspath dir_;
	int    inotifyfd_;
	int    watchfd_;
	int    cancelpipe_[2];

public:
	impl(const fspath& dir)
	    : dir_{ dir }
	    , inotifyfd_{}
	    , watchfd_{}
	    , cancelpipe_{}
	{
		this->inotifyfd_ = inotify_init();
		if (this->inotifyfd_ == -1)
		{
			ZOO_THROW_EXCEPTION(system_exception{} << error_opname{ "inotify_init" });
		}

		this->watchfd_ = inotify_add_watch(this->inotifyfd_, this->dir_.c_str(), IN_ALL_EVENTS & ~(IN_MODIFY | IN_ACCESS));
		if (this->watchfd_ == -1)
		{
			close(this->inotifyfd_);
			ZOO_THROW_EXCEPTION(system_exception{} << error_opname{ "inotify_add_watch" } << error_path{ this->dir_ });
		}

		if (pipe(this->cancelpipe_))
		{
			close(this->inotifyfd_);
			ZOO_THROW_EXCEPTION(system_exception{} << error_opname{ "pipe" });
		}

		zlog(debug, "watching {}", this->dir_);
	}

	~impl() noexcept
	{
		close(this->inotifyfd_);
		close(this->cancelpipe_[0]);
		close(this->cancelpipe_[1]);
	}

	std::vector<direntry> watch()
	{
		auto result = std::vector<direntry>{};

		auto rfds = fd_set{};
		FD_ZERO(&rfds);
		FD_SET(this->inotifyfd_, &rfds);
		FD_SET(this->cancelpipe_[0], &rfds);

		const auto maxfd = std::max(this->inotifyfd_, this->cancelpipe_[0]);

		const auto rc = ::select(maxfd + 1, &rfds, 0, 0, nullptr);
		if (rc < 0)
		{
			ZOO_THROW_EXCEPTION(system_exception{} << error_opname{ "inotify:select" });
		}
		else if (rc == 0)
		{
			// time out?
			return result;
		}
		else if (FD_ISSET(this->cancelpipe_[0], &rfds))
		{
			ZOO_THROW_EXCEPTION(interrupted_exception{});
		}
		else if (!FD_ISSET(this->inotifyfd_, &rfds))
		{
			return result;
		}

		char       buf[10 * (sizeof(inotify_event) + NAME_MAX + 1)];
		const auto size = ::read(this->inotifyfd_, buf, sizeof(buf));
		if (size < 0)
		{
			ZOO_THROW_EXCEPTION(system_exception{} << error_opname{ "inotify:read" });
		}
		else if (static_cast<size_t>(size) < sizeof(inotify_event))
		{
			ZOO_THROW_EXCEPTION(should_not_happen_exception{} << error_mesg{ "inotify read count too small" });
		}

		auto eventIndex            = int{ 0 };
		auto createdFileEventIndex = int{ -1 };
		auto createdFile           = std::optional<std::string>{};
		for (char* ptr = buf; ptr < buf + size; ++eventIndex)
		{
			boost::this_thread::interruption_point();

			const inotify_event* e = reinterpret_cast<inotify_event*>(ptr);
			ptr += sizeof(inotify_event) + e->len;

			if (e->len)
			{
				zlog(trace,
				     "inotify_event: name={0}, events={1}, idx={2}",
				     fmt::streamed(std::quoted(e->name)),
				     describe_flags(e->mask),
				     eventIndex);
			}
			else
			{
				zlog(trace, "inotify_event: name={0}, events={1}, idx={2}", "null", describe_flags(e->mask), eventIndex);
			}

			if (e->mask & IN_DELETE_SELF)
			{
				ZOO_THROW_EXCEPTION(exception(fmt::format("The watched directory {} was removed", this->dir_)));
			}

			if (e->mask & IN_MOVE_SELF)
			{
				ZOO_THROW_EXCEPTION(exception(fmt::format("The watched directory {} was moved", this->dir_)));
			}

			if (e->wd == this->watchfd_ && e->len)
			{
				if (boost::filesystem::is_directory(this->dir_ / e->name))
				{
					continue;
				}

				auto name = std::optional<std::string>{};

				if (e->mask & (IN_MOVED_TO | IN_CLOSE_WRITE))
				{
					name = e->name;
				}
				else if (e->mask == IN_CREATE)
				{
					createdFile           = e->name;
					createdFileEventIndex = eventIndex;
				}
				else if (e->mask == IN_OPEN)
				{
					if (eventIndex && eventIndex == createdFileEventIndex + 1 && createdFile && createdFile.value() == e->name)
					{
						createdFile           = std::nullopt;
						createdFileEventIndex = -1;
					}
				}
				else if (e->mask == IN_DELETE)
				{
					if (eventIndex && eventIndex == createdFileEventIndex + 1 && createdFile && createdFile.value() != e->name)
					{
						// this IN_DELETE immediately follows an IN_CREATE
						// consider this as a rename to the created file
						name                  = createdFile.value();
						createdFile           = std::nullopt;
						createdFileEventIndex = -1;
					}
				}

				if (name)
				{
					const auto path = this->dir_ / name.value();
					try
					{
						result.push_back(access::get_direntry(path));
					}
					catch (const std::exception& e)
					{
						zlog(err, "Getting direntry for {} failed: {}", path, e);
						// Maybe the file has since been removed
						// Maybe we don't have read permission
						// In any case, just ignore the file
					}
				}
			}
		}
		return result;
	}

	void cancel()
	{
		auto c = char{};
		write(this->cancelpipe_[1], &c, 1);
	}
};

} // namespace local
} // namespace fs
} // namespace zoo
