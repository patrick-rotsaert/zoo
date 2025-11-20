//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/ifile_event_listener.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/message.h"
#include "zoo/spider/response_wrapper.hpp"

#include <memory>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API file_response final
{
	using response = http::response<http::basic_file_body<tracked_file>>;

public:
	static response_wrapper
	create(const request& req, const fs::path& doc_root, string_view path, std::unique_ptr<ifile_event_listener>&& event_listener);
	static response_wrapper create(const request& req, const fs::path& doc_root, string_view path);

	static response_wrapper create(const fs::path& doc_root, string_view path, std::unique_ptr<ifile_event_listener>&& event_listener);
	static response_wrapper create(const fs::path& doc_root, string_view path);
};

} // namespace spider
} // namespace zoo
