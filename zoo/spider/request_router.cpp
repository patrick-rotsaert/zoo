//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/request_router.h"
#include "zoo/spider/error_response.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

#include <boost/beast/http.hpp> //FIXME: include less code?
#include <boost/url/parse.hpp>

#include <fmt/ostream.h>

#include <iostream>

namespace zoo {
namespace spider {

class request_router::impl final
{
	struct route final
	{
		std::set<verb>                  methods;
		boost::regex                    pattern;
		request_router::request_handler handler;

		route(std::set<verb>&& methods, boost::regex&& pattern, request_router::request_handler&& handler)
		    : methods{ std::move(methods) }
		    , pattern{ std::move(pattern) }
		    , handler{ std::move(handler) }
		{
		}
	};

	std::vector<route> routes_;

public:
	response_wrapper route_request(request&& req, url_view&& url, string_view path)
	{
		const auto pos = path.find_first_not_of('/');
		if (pos != string_view::npos && pos > 0u)
		{
			path = path.substr(pos);
		}

		for (const auto& route : this->routes_)
		{
			if (route.methods.empty() || route.methods.count(req.method()) > 0u)
			{
				auto match = svmatch{};
				if (boost::regex_search(path.begin(), path.end(), match, route.pattern))
				{
					return route.handler(std::move(req), std::move(url), path, match);
				}
			}
		}

		return not_found::create(req);
	}

	response_wrapper route_request(request&& req)
	{
		ZOO_LOG(trace, "request:\n{}", fmt::streamed(req));

		auto url = boost::urls::parse_origin_form(req.target());
		if (url.has_error())
		{
			ZOO_LOG(err, "Failed to parse the target '{}': {}", req.target(), url.error().message());
			return bad_request::create(req);
		}
		else
		{
			const auto path = url.value().path();
			return this->route_request(std::move(req), std::move(url.value()), path);
		}
	}

	void add_route(std::set<verb>&& methods, boost::regex&& pattern, request_handler&& handler)
	{
		this->routes_.emplace_back(std::move(methods), std::move(pattern), std::move(handler));
	}

	void add_route(std::set<verb>&& methods, boost::regex&& pattern, const std::shared_ptr<request_router>& router)
	{
		this->routes_.emplace_back(
		    std::move(methods), std::move(pattern), [r = router](request&& req, url_view&& url, string_view path, const svmatch& match) {
			    assert(!match.empty());
			    // Let the router route the remainder of the path
			    const auto remainder = path.substr(match[0].second - path.begin());
			    return r->route_request(std::move(req), std::move(url), remainder);
		    });
	}
};

request_router::request_router()
    : pimpl_{ std::make_unique<impl>() }
{
}

request_router::~request_router() noexcept = default;

request_router::request_router(request_router&&) = default;

request_router& request_router::operator=(request_router&&) = default;

response_wrapper request_router::route_request(request&& req, url_view&& url, string_view path)
{
	return this->pimpl_->route_request(std::move(req), std::move(url), path);
}

response_wrapper request_router::handle_request(request&& req)
{
	return this->pimpl_->route_request(std::move(req));
}

void request_router::add_route(std::set<verb>&& methods, boost::regex&& pattern, request_handler&& handler)
{
	return this->pimpl_->add_route(std::move(methods), std::move(pattern), std::move(handler));
}

void request_router::add_route(std::set<verb>&& methods, boost::regex&& pattern, const std::shared_ptr<request_router>& router)
{
	return this->pimpl_->add_route(std::move(methods), std::move(pattern), router);
}

void request_router::add_route(verb method, boost::regex&& pattern, request_handler&& handler)
{
	return this->pimpl_->add_route({ method }, std::move(pattern), std::move(handler));
}

void request_router::add_route(verb method, boost::regex&& pattern, const std::shared_ptr<request_router>& router)
{
	return this->pimpl_->add_route({ method }, std::move(pattern), router);
}

void request_router::add_route(boost::regex&& pattern, request_handler&& handler)
{
	return this->pimpl_->add_route({}, std::move(pattern), std::move(handler));
}

void request_router::add_route(boost::regex&& pattern, const std::shared_ptr<request_router>& router)
{
	return this->pimpl_->add_route({}, std::move(pattern), router);
}

} // namespace spider
} // namespace zoo
