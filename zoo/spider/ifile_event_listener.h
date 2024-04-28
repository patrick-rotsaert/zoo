//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"

#include <boost/beast/core/file_base.hpp>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API ifile_event_listener
{
public:
	using file_mode  = boost::beast::file_mode;
	using error_code = boost::beast::error_code;

	ifile_event_listener();
	virtual ~ifile_event_listener();

	virtual void on_open(char const* path, file_mode mode, const error_code& ec)       = 0;
	virtual void on_close(const error_code& ec, std::uint64_t size, std::uint64_t pos) = 0;
	virtual void on_seek(std::uint64_t offset, const error_code& ec)                   = 0;
	virtual void on_read(void* buffer, std::size_t n, const error_code& ec)            = 0;
	virtual void on_write(const void* buffer, std::size_t n, const error_code& ec)     = 0;
};

} // namespace spider
} // namespace zoo
