//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/concepts.hpp"

#include <boost/beast/http/message_generator.hpp>
#include <boost/url/url_view.hpp>
#include <boost/json.hpp>
#include <boost/regex.hpp>

#include <memory>
#include <regex>
#include <set>
#include <functional>
#include <concepts>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API request_router final : public irequest_handler
{
	class impl;
	std::unique_ptr<impl> pimpl_;
	friend impl; // allow impl to call private method route_request

	using svmatch         = boost::match_results<string_view::const_iterator>;
	using request_handler = std::function<message_generator(request&& req, url_view&& url, string_view path, const svmatch& match)>;
	template<ConvertibleFromBoostJson T>
	using json_request_handler = std::function<
	    message_generator(request&& req, url_view&& url, string_view path, const svmatch& match, boost::json::result<T>&& data)>;

	message_generator handle_request(request&& req) override;
	message_generator route_request(request&& req, url_view&& url, string_view path);

public:
	request_router();
	~request_router() noexcept override;

	request_router(request_router&&);
	request_router& operator=(request_router&&);

	request_router(const request_router&)            = delete;
	request_router& operator=(const request_router&) = delete;

	void add_route(std::set<verb>&& methods, boost::regex&& pattern, request_handler&& handler);
	void add_route(std::set<verb>&& methods, boost::regex&& pattern, const std::shared_ptr<request_router>& router);

	void add_route(verb method, boost::regex&& pattern, request_handler&& handler);
	void add_route(verb method, boost::regex&& pattern, const std::shared_ptr<request_router>& router);

	void add_route(boost::regex&& pattern, request_handler&& handler);
	void add_route(boost::regex&& pattern, const std::shared_ptr<request_router>& router);

	template<ConvertibleFromBoostJson T>
	void add_json_route(std::set<verb>&& methods, boost::regex&& pattern, json_request_handler<T>&& handler)
	{
		this->add_route(
		    std::move(methods),
		    std::move(pattern),
		    [handler = std::move(handler)](request&& req, url_view&& url, string_view path, const svmatch& match) -> message_generator {
			    using result = boost::json::result<T>;

			    auto       ec = std::error_code{};
			    const auto jv = boost::json::parse(req.body(), ec);
			    if (ec)
			    {
				    return handler(std::move(req), std::move(url), path, match, result{ result::in_place_error, ec });
			    }
			    else
			    {
				    return handler(std::move(req), std::move(url), path, match, boost::json::try_value_to<T>(jv));
			    }
		    });
	}

	template<ConvertibleFromBoostJson T>
	void add_json_route(verb method, boost::regex&& pattern, json_request_handler<T>&& handler)
	{
		this->add_json_route<T>(std::set<verb>{ method }, std::move(pattern), std::move(handler));
	}

	template<ConvertibleFromBoostJson T>
	void add_json_route(boost::regex&& pattern, json_request_handler<T>&& handler)
	{
		this->add_json_route<T>(std::set<verb>{}, std::move(pattern), std::move(handler));
	}
};

} // namespace spider
} // namespace zoo
