//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/request_router2.h"
#include "zoo/spider/exception.h"
#include "zoo/spider/error_response.h" //@@
#include "zoo/spider/path.h"
#include "zoo/spider/pathspec.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/misc/throw_exception.h"

#include <boost/beast/http/message_generator.hpp>

#include <boost/beast/http.hpp> //FIXME: include less code?
#include <boost/url/parse.hpp>

#include <fmt/ostream.h>

#include <iostream>

namespace zoo {
namespace spider {

class request_router2::impl final
{
	struct route final
	{
		operation                        op;
		request_router2::request_handler handler;

		route(operation op, request_router2::request_handler handler)
		    : op{ std::move(op) }
		    , handler{ std::move(handler) }
		{
		}
	};

	std::vector<route> routes_;

public:
	message_generator route_request(request&& req, url_view&& url, path&& p)
	{
		for (const auto& route : this->routes_)
		{
			if (route.op.method == req.method())
			{
				auto param_map = route.op.path.match(p);
				if (param_map.has_value())
				{
					return route.handler(std::move(req), std::move(url), std::move(param_map.value()));
				}
			}
		}

		return not_found::create(req); //@@
		                               // ZOO_THROW_EXCEPTION(not_found_exception{});
	}

	message_generator route_request(request&& req)
	{
		ZOO_LOG(trace, "request:\n{}", fmt::streamed(req));

		auto url = boost::urls::parse_origin_form(req.target());
		if (url.has_error())
		{
			ZOO_LOG(err, "Failed to parse the target '{}': {}", req.target(), url.error().message());
			return bad_request::create(req); //@@
			                                 // ZOO_THROW_EXCEPTION(bad_request_exception{});
		}

		return this->route_request(std::move(req), std::move(url.value()), path{ url.value().path() });
	}

	void add_route(operation op, request_handler handler)
	{
		this->routes_.emplace_back(std::move(op), std::move(handler));
	}
};

request_router2::request_router2()
    : pimpl_{ std::make_unique<impl>() }
{
}

request_router2::~request_router2() noexcept                   = default;
request_router2::request_router2(request_router2&&)            = default;
request_router2& request_router2::operator=(request_router2&&) = default;

message_generator request_router2::handle_request(request&& req)
{
	return this->pimpl_->route_request(std::move(req));
}

void request_router2::add_route(operation op, request_handler handler)
{
	return this->pimpl_->add_route(std::move(op), std::move(handler));
}

} // namespace spider
} // namespace zoo
