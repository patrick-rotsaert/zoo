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
#include <set>

#include <windows.h>

namespace zoo {
namespace fs {
namespace local {

class watcher::impl final
{
	fspath                 dir_;
	HANDLE                 directory_handle_;
	OVERLAPPED             overlapped_;
	HANDLE                 cancel_event_;
	std::set<std::wstring> added_files_;
	bool                   read_pending_;

public:
	impl(const fspath& dir)
	    : dir_{ dir }
	    , directory_handle_{}
	    , overlapped_{}
	    , cancel_event_{}
	    , added_files_{}
	    , read_pending_{}
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

		ZOO_LOG(debug, "watching {}", this->dir_);
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

		auto&& add_entry_lamdba = [&](const fspath& path) {
			try
			{
				result.push_back(access::get_direntry(path));
			}
			catch (const std::exception& e)
			{
				ZOO_LOG(err, "Getting direntry for {} failed: {}", path, e);
				// Maybe the file has since been removed
				// Maybe we don't have read permission
				// In any case, just ignore the file
			}
		};

		auto&& check_added_files_lambda = [&]() {
			for (auto it = this->added_files_.begin(), end = this->added_files_.end(); it != end;)
			{
				const auto& filename = *it;
				const auto  path     = (this->dir_ / filename).make_preferred();
				const auto  handle   = CreateFileW(path.c_str(),                 // lpFileName
                                                GENERIC_READ | GENERIC_WRITE, // dwDesiredAccess
                                                0,                            // dwShareMode
                                                NULL,                         // lpSecurityAttributes
                                                OPEN_EXISTING,                // dwCreationDisposition
                                                FILE_ATTRIBUTE_NORMAL,        // dwFlagsAndAttributes
                                                NULL                          // hTemplateFile
                );
				if (handle == INVALID_HANDLE_VALUE)
				{
					const auto err = GetLastError();
					if (err == ERROR_SHARING_VIOLATION)
					{
						// File still open by other process
						ZOO_LOG(trace, "sharing violation for {}", path);
						++it;
					}
					else if (err == ERROR_FILE_NOT_FOUND)
					{
						// File was removed
						ZOO_LOG(trace, "file {} no longer exists", path);
						it = this->added_files_.erase(it);
					}
					else
					{
						ZOO_LOG(trace, "opening {} failed with error {}", path, err);
						++it;
					}
				}
				else
				{
					ZOO_LOG(trace, "opening {} succeeded", path);
					CloseHandle(handle);
					add_entry_lamdba(path);
					it = this->added_files_.erase(it);
				}
			}
		};

		char buffer[1024];

		auto&& read_changes_lambda = [&]() {
			if (this->read_pending_)
			{
				return;
			}

			const auto success = ReadDirectoryChangesW(this->directory_handle_,                                      // hDirectory
			                                           buffer,                                                       // lpBuffer
			                                           sizeof(buffer),                                               // nBufferLength
			                                           FALSE,                                                        // bWatchSubtree
			                                           FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, // dwNotifyFilter
			                                           NULL,                                                         // lpBytesReturned
			                                           &this->overlapped_,                                           // lpOverlapped
			                                           NULL                                                          // lpCompletionRoutine
			);
			ZOO_LOG(trace, "ReadDirectoryChangesW: {}", success);

			if (!success)
			{
				ZOO_THROW_EXCEPTION(std::system_error{
				    static_cast<int>(::GetLastError()), std::system_category(), this->dir_.string() + ": ReadDirectoryChangesW" });
			}

			this->read_pending_ = true;
		};

		read_changes_lambda();

		for (;;)
		{
			boost::this_thread::interruption_point();

			HANDLE     wait_handles[]       = { this->overlapped_.hEvent, this->cancel_event_ };
			const auto timeout_milliseconds = DWORD{ this->added_files_.empty() ? INFINITE : 50 };

			const auto wait_result = WaitForMultipleObjects(2, wait_handles, FALSE, timeout_milliseconds);

			if (wait_result == WAIT_OBJECT_0)
			{
				this->read_pending_ = false;

				auto bytes_returned = DWORD{};

				const auto success = GetOverlappedResult(this->directory_handle_, &this->overlapped_, &bytes_returned, FALSE);
				ZOO_LOG(trace, "GetOverlappedResult: {}, bytes_returned: {}", success, bytes_returned);

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

					const auto name_length = notifyInfo->FileNameLength / sizeof(*notifyInfo->FileName);
					const auto filename    = std::wstring{ notifyInfo->FileName, name_length };
					const auto path        = (this->dir_ / filename).make_preferred();

					switch (notifyInfo->Action)
					{
					case FILE_ACTION_ADDED:
						ZOO_LOG(trace, "   added: {}", path);
						this->added_files_.insert(filename);
						break;
					case FILE_ACTION_REMOVED:
						ZOO_LOG(trace, " removed: {}", path);
						this->added_files_.erase(filename);
						break;
					case FILE_ACTION_MODIFIED:
						ZOO_LOG(trace, "modified: {}", path);
						this->added_files_.insert(filename);
						break;
					case FILE_ACTION_RENAMED_OLD_NAME:
						ZOO_LOG(trace, "ren from: {}", path);
						this->added_files_.erase(filename);
						break;
					case FILE_ACTION_RENAMED_NEW_NAME:
						ZOO_LOG(trace, "ren   to: {}", path);
						this->added_files_.erase(filename);
						add_entry_lamdba(path);
						break;
					default:
						ZOO_LOG(warn, "Unknown action {}", notifyInfo->Action);
						break;
					}

					if (notifyInfo->NextEntryOffset == 0)
					{
						break;
					}

					notifyInfo =
					    reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(notifyInfo) + notifyInfo->NextEntryOffset);
				}

				check_added_files_lambda();

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
				check_added_files_lambda();

				if (!result.empty())
				{
					break;
				}
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
