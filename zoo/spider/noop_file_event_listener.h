//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/ifile_event_listener.h"

namespace zoo {
namespace spider {

class ZOO_SPIDER_API noop_file_event_listener : public ifile_event_listener
{
public:
	noop_file_event_listener();
	~noop_file_event_listener() override;

	void on_open(char const* path, file_mode mode, const error_code& ec) override;
	void on_close(const error_code& ec, std::uint64_t size, std::uint64_t pos) override;
	void on_seek(std::uint64_t offset, const error_code& ec) override;
	void on_read(void* buffer, std::size_t n, const error_code& ec) override;
	void on_write(const void* buffer, std::size_t n, const error_code& ec) override;
};

} // namespace spider
} // namespace zoo
