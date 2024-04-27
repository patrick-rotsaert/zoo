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
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

#include <boost/thread/interruption.hpp>

#include <filesystem>
#include <optional>

#include <windows.h>

namespace zoo {
namespace fs {
namespace local {

class watcher::impl final
{
	fspath dir_;
	HANDLE directory_handle_;

public:
	impl(const fspath& dir)
	    : dir_{ dir }
	    , directory_handle_{}
	{
		this->directory_handle_ = CreateFileW(dir.c_str(),
		                                      FILE_LIST_DIRECTORY,
		                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		                                      NULL,
		                                      OPEN_EXISTING,
		                                      FILE_FLAG_BACKUP_SEMANTICS /*| FILE_FLAG_OVERLAPPED*/,
		                                      NULL);

		if (this->directory_handle_ == INVALID_HANDLE_VALUE)
		{
			ZOO_THROW_EXCEPTION(
			    std::system_error{ static_cast<int>(::GetLastError()), std::system_category(), dir.string() + ": CreateFileW" });
		}
	}

	~impl() noexcept
	{
		CloseHandle(this->directory_handle_);
	}

	std::vector<direntry> watch()
	{
		auto result = std::vector<direntry>{};

		char buffer[1024];
		auto bytes_returned = DWORD{};

		zlog(debug, "watching {}", this->dir_);

		auto success = ReadDirectoryChangesW(this->directory_handle_,                                      // hDirectory
		                                     buffer,                                                       // lpBuffer
		                                     sizeof(buffer),                                               // nBufferLength
		                                     FALSE,                                                        // bWatchSubtree
		                                     FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, // dwNotifyFilter
		                                     &bytes_returned,                                              // lpBytesReturned
		                                     NULL,                                                         // lpOverlapped
		                                     NULL                                                          // lpCompletionRoutine
		);
		zlog(trace, "ReadDirectoryChangesW: {}, bytes_returned={}", success, bytes_returned);

		if (!success)
		{
			ZOO_THROW_EXCEPTION(std::system_error{
			    static_cast<int>(::GetLastError()), std::system_category(), this->dir_.string() + ": ReadDirectoryChangesW" });
		}

		if (bytes_returned == 0)
		{
			return result;
		}

		auto notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

		while (notifyInfo)
		{
			boost::this_thread::interruption_point();

			zlog(trace, "notifyInfo->Action = {}", notifyInfo->Action);

			if (notifyInfo->Action == FILE_ACTION_MODIFIED || notifyInfo->Action == FILE_ACTION_ADDED ||
			    notifyInfo->Action == FILE_ACTION_RENAMED_NEW_NAME)
			{
				const auto filename = std::wstring{ notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(*notifyInfo->FileName) };
				result.push_back(access::get_direntry(dir_ / filename));
			}

			// Move to the next notification record
			if (notifyInfo->NextEntryOffset == 0)
			{
				break;
			}

			notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(notifyInfo) + notifyInfo->NextEntryOffset);
		}

		return result;
	}

	void cancel()
	{
		// TODO
	}
};

} // namespace local
} // namespace fs
} // namespace zoo
