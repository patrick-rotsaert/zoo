//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/rest/operation.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/aliases.h"

#include <boost/beast/http/message_generator.hpp>
#include <boost/url/url_view.hpp>

#include <functional>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API rest_router final : public irequest_handler
{
	using request_handler = std::function<message_generator(request&& req, url_view&& url, path_spec::param_map&& param)>;

	message_generator handle_request(request&& req) override;
	message_generator route_request(request&& req, url_view&& url, path&& p);

	struct route final
	{
		rest_operation  op;
		request_handler handler;

		route(rest_operation op, request_handler handler);
	};

	std::vector<route> routes_;

public:
	rest_router();
	rest_router(rest_router&&);
	rest_router(const rest_router&) = delete;

	~rest_router() noexcept override;

	rest_router& operator=(rest_router&&);
	rest_router& operator=(const rest_router&) = delete;

	void add_route(rest_operation op, request_handler handler);
};

} // namespace spider
} // namespace zoo
