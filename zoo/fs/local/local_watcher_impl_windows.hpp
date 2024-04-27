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
	fspath     dir_;
	HANDLE     directory_handle_;
	OVERLAPPED overlapped_;
	HANDLE     cancel_event_;

public:
	impl(const fspath& dir)
	    : dir_{ dir }
	    , directory_handle_{}
	    , overlapped_{}
	{
		this->directory_handle_ = CreateFileW(dir.c_str(),
		                                      FILE_LIST_DIRECTORY,
		                                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		                                      NULL,
		                                      OPEN_EXISTING,
		                                      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		                                      NULL);

		if (this->directory_handle_ == INVALID_HANDLE_VALUE)
		{
			ZOO_THROW_EXCEPTION(
			    std::system_error{ static_cast<int>(::GetLastError()), std::system_category(), dir.string() + ": CreateFileW" });
		}

		this->overlapped_.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (this->overlapped_.hEvent == NULL)
		{
			ZOO_THROW_EXCEPTION(std::system_error{ static_cast<int>(::GetLastError()), std::system_category(), "CreateEvent" });
		}

		this->cancel_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (this->cancel_event_ == NULL)
		{
			ZOO_THROW_EXCEPTION(std::system_error{ static_cast<int>(::GetLastError()), std::system_category(), "CreateEvent" });
		}
	}

	~impl() noexcept
	{
		CloseHandle(this->directory_handle_);
		CloseHandle(this->overlapped_.hEvent);
		CloseHandle(this->cancel_event_);
	}

	std::vector<direntry> watch()
	{
		auto result = std::vector<direntry>{};

		char buffer[1024];

		auto&& read_changes_lambda = [&]() {
			const auto success = ReadDirectoryChangesW(this->directory_handle_,                                      // hDirectory
			                                           buffer,                                                       // lpBuffer
			                                           sizeof(buffer),                                               // nBufferLength
			                                           FALSE,                                                        // bWatchSubtree
			                                           FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, // dwNotifyFilter
			                                           NULL,                                                         // lpBytesReturned
			                                           &this->overlapped_,                                           // lpOverlapped
			                                           NULL                                                          // lpCompletionRoutine
			);
			zlog(trace, "ReadDirectoryChangesW: {}", success);

			if (!success)
			{
				ZOO_THROW_EXCEPTION(std::system_error{
				    static_cast<int>(::GetLastError()), std::system_category(), this->dir_.string() + ": ReadDirectoryChangesW" });
			}

			zlog(debug, "watching {}", this->dir_);
		};

		read_changes_lambda();

		for (;;)
		{
			boost::this_thread::interruption_point();

			//const auto wait_result = WaitForSingleObject(this->overlapped_.hEvent, 0);

			HANDLE     wait_handles[] = { this->overlapped_.hEvent, this->cancel_event_ };
			const auto wait_result    = WaitForMultipleObjects(2, wait_handles, FALSE, INFINITE);

			if (wait_result == WAIT_OBJECT_0)
			{
				auto bytes_returned = DWORD{};

				const auto success = GetOverlappedResult(this->directory_handle_, &this->overlapped_, &bytes_returned, FALSE);
				zlog(trace, "GetOverlappedResult: {}, bytes_returned: {}", success, bytes_returned);

				if (!success)
				{
					ZOO_THROW_EXCEPTION(std::system_error{
					    static_cast<int>(::GetLastError()), std::system_category(), this->dir_.string() + ": GetOverlappedResult" });
				}

				if (bytes_returned == 0)
				{
					break;
				}

				auto notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

				while (notifyInfo)
				{
					boost::this_thread::interruption_point();

					zlog(trace, "notifyInfo->Action = {}", notifyInfo->Action);

					if (notifyInfo->Action == FILE_ACTION_MODIFIED || notifyInfo->Action == FILE_ACTION_ADDED ||
					    notifyInfo->Action == FILE_ACTION_RENAMED_NEW_NAME)
					{
						const auto name_length = notifyInfo->FileNameLength / sizeof(*notifyInfo->FileName);
						const auto filename    = std::wstring{ notifyInfo->FileName, name_length };
						result.push_back(access::get_direntry(dir_ / filename));
					}

					if (notifyInfo->NextEntryOffset == 0)
					{
						break;
					}

					notifyInfo =
					    reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(notifyInfo) + notifyInfo->NextEntryOffset);
				}

				if (!result.empty())
				{
					break;
				}

				read_changes_lambda();
			}
			else if (wait_result == WAIT_OBJECT_0 + 1)
			{
				ZOO_THROW_EXCEPTION(interrupted_exception{});
			}
			else if (wait_result == WAIT_TIMEOUT)
			{
				zlog(warn, "Timeout occurred, should not happen.");
			}
			else
			{
				ZOO_THROW_EXCEPTION(
				    std::system_error{ static_cast<int>(::GetLastError()), std::system_category(), "WaitForMultipleObjects" });
			}
		}

		return result;
	}

	void cancel()
	{
		SetEvent(this->cancel_event_);
	}
};

} // namespace local
} // namespace fs
} // namespace zoo
