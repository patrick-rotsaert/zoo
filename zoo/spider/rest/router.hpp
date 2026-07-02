//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/exception.h"
#include "zoo/spider/messages/json_response.h"
#include "zoo/spider/rest/concepts.hpp"
#include "zoo/spider/rest/operation.h"
#include "zoo/spider/rest/path.h"
#include "zoo/spider/rest/pathspec.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/misc/throw_exception.h"

#include <boost/beast/http.hpp> //FIXME: include less code?
#include <boost/url/parse.hpp>
#include <boost/url/url_view.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cstddef>
#include <functional>

namespace zoo {
namespace spider {

template<IsValidErrorType DefaultErrorType>
class rest_router final : public irequest_handler
{
	using request_handler = std::function<response_wrapper(request&& req, url_view&& url, path_spec::param_map&& param)>;

	struct route final
	{
		rest_operation  op;
		request_handler handler;
	};

public:
	rest_router()                   = default;
	rest_router(rest_router&&)      = default;
	rest_router(const rest_router&) = delete;

	~rest_router() noexcept override = default;

	rest_router& operator=(rest_router&&)      = default;
	rest_router& operator=(const rest_router&) = delete;

	void add_route(rest_operation op, request_handler handler)
	{
		// Keep routes ordered most-specific-first so that route_request can return on the first match.
		// This ordering work happens only while routes are added (typically at startup), keeping the
		// per-request path cheap. upper_bound + insert preserves registration order among routes of
		// equal specificity (e.g. same path, different methods).
		const auto pos = std::upper_bound(routes_.begin(), routes_.end(), op, [](const rest_operation& value, const route& r) {
			return path_precedes(value.path, r.op.path);
		});
		routes_.emplace(pos, std::move(op), std::move(handler));
	}

private:
	response_wrapper handle_request(request&& req) override
	{
		auto url = boost::urls::parse_origin_form(req.target());
		if (url.has_error())
		{
			return json_response::create(
			    req,
			    status::bad_request,
			    DefaultErrorType::create(static_cast<int>(status::bad_request),
			                             fmt::format("Failed to parse the target '{}': {}", req.target(), url.error().message())));
		}

		return route_request(std::move(req), std::move(url.value()), path{ url.value().path() });
	}

	// Strict weak ordering used to keep routes_ sorted so the first match is the most specific one.
	// Routes are grouped by segment count (only same-length specs can match a given path), and within
	// a group a spec whose first differing segment is literal precedes one whose segment is a
	// {parameter}. So a literal route (/users/me) is ordered before a parameter route (/users/{id})
	// and is therefore matched first regardless of registration order.
	static bool path_precedes(const path_spec& a, const path_spec& b)
	{
		const auto& sa = a.segments();
		const auto& sb = b.segments();
		if (sa.size() != sb.size())
		{
			return sa.size() < sb.size();
		}
		for (std::size_t i = 0; i < sa.size(); ++i)
		{
			if (sa[i].is_parameter != sb[i].is_parameter)
			{
				return !sa[i].is_parameter; // literal before parameter
			}
		}
		return false;
	}

	response_wrapper route_request(request&& req, url_view&& url, path&& p)
	{
		// routes_ is kept most-specific-first by add_route, so the first matching route is the most
		// specific one and we can return immediately on the first match.
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
			return json_response::create(
			    req,
			    status::method_not_allowed,
			    DefaultErrorType::create(static_cast<int>(status::method_not_allowed),
			                             fmt::format("Method '{}' not allowed for target '{}'", req.method_string(), p.to_string())));
		}
		else
		{
			return json_response::create(
			    req,
			    status::not_found,
			    DefaultErrorType::create(static_cast<int>(status::not_found), fmt::format("Target not found: '{}'", p.to_string())));
		}
	}

	std::vector<route> routes_{};
};

} // namespace spider
} // namespace zoo
