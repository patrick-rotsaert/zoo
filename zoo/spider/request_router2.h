//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/pathspec.h"
#include "zoo/spider/aliases.h"

#include <boost/beast/http/message_generator.hpp>
#include <boost/url/url_view.hpp>

#include <memory>
#include <functional>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API request_router2 final : public irequest_handler
{
	class impl;
	std::unique_ptr<impl> pimpl_;

	using request_handler = std::function<message_generator(request&& req, url_view&& url, path_spec::param_map&& param)>;

	message_generator handle_request(request&& req) override;

public:
	request_router2();
	~request_router2() noexcept override;

	request_router2(request_router2&&);
	request_router2& operator=(request_router2&&);

	request_router2(const request_router2&)            = delete;
	request_router2& operator=(const request_router2&) = delete;

	void add_route(verb method, path_spec&& path, request_handler&& handler);
};

} // namespace spider
} // namespace zoo
