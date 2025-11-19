//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/rest/router.h"
#include "zoo/spider/rest/path.h"
#include "zoo/spider/rest/pathspec.h"
#include "zoo/spider/exception.h"
#include "zoo/spider/error_response.h" //@@
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

rest_router::route::route(operation op, request_handler handler)
    : op{ std::move(op) }
    , handler{ std::move(handler) }
{
}

rest_router::rest_router()
    : routes_{}
{
}

rest_router::rest_router(rest_router&&) = default;

rest_router::~rest_router() noexcept = default;

rest_router& rest_router::operator=(rest_router&&) = default;

message_generator rest_router::handle_request(request&& req)
{
	ZOO_LOG(trace, "request:\n{}", fmt::streamed(req));

	auto url = boost::urls::parse_origin_form(req.target());
	if (url.has_error())
	{
		ZOO_LOG(err, "Failed to parse the target '{}': {}", req.target(), url.error().message());
		return bad_request::create(req); //@@
	}

	return route_request(std::move(req), std::move(url.value()), path{ url.value().path() });
}

message_generator rest_router::route_request(request&& req, url_view&& url, path&& p)
{
	auto found = false;

	for (const auto& route : routes_)
	{
		if (route.op.method == req.method())
		{
			auto param_map = route.op.path.match(p);
			if (param_map.has_value())
			{
				return route.handler(std::move(req), std::move(url), std::move(param_map.value()));
			}
		}
		else if (route.op.path.match(p).has_value())
		{
			found = true;
		}
	}

	if (found)
	{
		return method_not_allowed::create(req); //@@
	}
	else
	{
		return not_found::create(req); //@@
	}
}

void rest_router::add_route(operation op, request_handler handler)
{
	routes_.emplace_back(std::move(op), std::move(handler));
}

} // namespace spider
} // namespace zoo
