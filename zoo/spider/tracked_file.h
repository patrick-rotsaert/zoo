//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/ifile_event_listener.h"
#include "zoo/spider/aliases.h"
#include "zoo/common/api.h"

#include <boost/beast/core/file.hpp>

#include <memory>

namespace zoo {
namespace spider {

// Wrapper around beast::file, for the purpose of tracking file read progress
class ZOO_EXPORT tracked_file final
{
	beast::file                           file_;
	std::unique_ptr<ifile_event_listener> event_listener_;

public:
	using native_handle_type = beast::file::native_handle_type;
	using file_mode          = beast::file_mode;
	using error_code         = beast::error_code;

	~tracked_file();

	tracked_file() = default;

	explicit tracked_file(std::unique_ptr<ifile_event_listener>&& event_listener);

	tracked_file(tracked_file&& other)      = default;
	tracked_file(const tracked_file& other) = delete;

	tracked_file& operator=(tracked_file&& other) = default;
	tracked_file& operator=(const tracked_file& other) = delete;

	native_handle_type native_handle();

	void native_handle(native_handle_type fd);

	bool is_open() const;

	void close(error_code& ec);

	void open(char const* path, file_mode mode, error_code& ec);

	std::uint64_t size(error_code& ec) const;

	std::uint64_t pos(error_code& ec);

	void seek(std::uint64_t offset, error_code& ec);

	std::size_t read(void* buffer, std::size_t n, error_code& ec);

	std::size_t write(void const* buffer, std::size_t n, error_code& ec);
};

} // namespace spider
} // namespace zoo
