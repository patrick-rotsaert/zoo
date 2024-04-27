//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/sftp_watcher.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include <map>

namespace zoo {
namespace fs {
namespace sftp {

class watcher::impl final
{
	fspath                          dir_;
	std::uint32_t                   scan_interval_ms_;
	std::shared_ptr<iaccess>        access_;
	std::shared_ptr<iinterruptor>   interruptor_;
	std::map<std::string, direntry> files_;

public:
	impl(const fspath& dir, std::uint32_t scan_interval_ms, std::shared_ptr<iaccess> access, std::shared_ptr<iinterruptor> interruptor)
	    : dir_{ dir }
	    , scan_interval_ms_{ scan_interval_ms }
	    , access_{ access }
	    , interruptor_{ interruptor }
	    , files_{ list_files() }
	{
		zlog(debug, "watching {}", this->dir_);
		zlog(trace, "initial file count = {}", this->files_.size());
	}

	std::vector<direntry> watch()
	{
		std::vector<direntry> result;

		zlog(trace, "wait for interruption during {} ms", this->scan_interval_ms_);
		if (this->interruptor_->wait_for_interruption(std::chrono::milliseconds{ this->scan_interval_ms_ }))
		{
			ZOO_THROW_EXCEPTION(interrupted_exception{});
		}

		auto files = this->list_files();
		zlog(trace, "current file count = {}, previous file count = {}", files.size(), this->files_.size());

		for (const auto& pair : files)
		{
			const auto& file = pair.second;
			const auto  it   = this->files_.find(file.name);
			if (it == this->files_.end())
			{
				result.push_back(file);
			}
		}

		this->files_.swap(files);

		return result;
	}

	void cancel()
	{
		// TODO
	}

private:
	std::map<std::string, direntry> list_files()
	{
		auto result = std::map<std::string, direntry>{};

		// get all entries in directory
		const auto files = this->access_->ls(this->dir_);

		// convert to map
		for (auto& entry : files)
		{
			auto name = entry.name;
			result.emplace(std::move(name), std::move(entry));
		}

		return result;
	}
};

watcher::watcher(const fspath&                 dir,
                 std::uint32_t                 scan_interval_ms,
                 std::shared_ptr<iaccess>      access,
                 std::shared_ptr<iinterruptor> interruptor)
    : pimpl_{ std::make_unique<impl>(dir, scan_interval_ms, access, interruptor) }
{
}

watcher::~watcher() noexcept
{
}

std::vector<direntry> watcher::watch()
{
	return this->pimpl_->watch();
}

void watcher::cancel()
{
	return this->pimpl_->cancel();
}

} // namespace sftp
} // namespace fs
} // namespace zoo
