//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/tracked_file.h"
#include "zoo/spider/ifile_event_listener.h"
#include "zoo/common/logging/logging.h"

namespace zoo {
namespace spider {

tracked_file::~tracked_file()
{
	try
	{
		if (this->is_open())
		{
			auto ec = error_code{};
			this->close(ec);
		}
	}
	catch (const std::exception& e)
	{
		ZOO_LOG(warn, "{}", e.what());
	}
}

tracked_file::tracked_file(std::unique_ptr<ifile_event_listener>&& event_listener)
    : event_listener_{ std::move(event_listener) }
{
}

tracked_file::native_handle_type tracked_file::native_handle()
{
	return this->file_.native_handle();
}

void tracked_file::native_handle(native_handle_type fd)
{
	return this->file_.native_handle(fd);
}

bool tracked_file::is_open() const
{
	return this->file_.is_open();
}

void tracked_file::close(error_code& ec)
{
	if (this->event_listener_)
	{
		const auto pos  = this->pos(ec);
		const auto size = this->size(ec);
		this->file_.close(ec);
		this->event_listener_->on_close(ec, size, pos);
	}
	else
	{
		return this->file_.close(ec);
	}
}

void tracked_file::open(const char* path, file_mode mode, error_code& ec)
{
	this->file_.open(path, mode, ec);
	if (this->event_listener_)
	{
		this->event_listener_->on_open(path, mode, ec);
	}
}

uint64_t tracked_file::size(error_code& ec) const
{
	return this->file_.size(ec);
}

uint64_t tracked_file::pos(error_code& ec)
{
	return this->file_.pos(ec);
}

void tracked_file::seek(uint64_t offset, error_code& ec)
{
	this->file_.seek(offset, ec);
	if (this->event_listener_)
	{
		this->event_listener_->on_seek(offset, ec);
	}
}

std::size_t tracked_file::read(void* buffer, std::size_t n, error_code& ec)
{
	n = this->file_.read(buffer, n, ec);
	if (this->event_listener_)
	{
		this->event_listener_->on_read(buffer, n, ec);
	}
	return n;
}

std::size_t tracked_file::write(const void* buffer, std::size_t n, error_code& ec)
{
	n = this->file_.write(buffer, n, ec);
	if (this->event_listener_)
	{
		this->event_listener_->on_write(buffer, n, ec);
	}
	return n;
}

} // namespace spider
} // namespace zoo
