//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

// Code in this file is inspired by https://github.com/mhekkel/libzeep

// Copyright Maarten L. Hekkelman, Radboud University 2008-2013.
//        Copyright Maarten L. Hekkelman, 2014-2022
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/handler.hpp"
#include "zoo/spider/concepts.hpp"
#include "zoo/spider/parameters.h"
#include "zoo/spider/request_router2.h"
#include "zoo/spider/openapi.h"
#include "zoo/spider/error_response.h"
#include "zoo/spider/response_wrapper.hpp"
#include "zoo/spider/json_response.h"
#include "zoo/spider/empty_response.h"
#include "zoo/spider/exception.h"

#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/misc/demangled_type_name.hpp"

#include <boost/json.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/exception/errinfo_nested_exception.hpp>

#include <fmt/format.h>

#include <type_traits>
#include <concepts>

#include <tuple>
#include <string>
#include <array>
#include <iomanip>

namespace zoo {
namespace spider {

template<IsValidErrorType ErrorType>
class controller2
{
public:
	using p = parameters::p;

	explicit controller2(std::shared_ptr<request_router2> router, openapi_settings settings)
	    : router_{ std::move(router) }
	    , oas_{ std::move(settings) }
	{
	}

	virtual ~controller2()
	{
	}

	const std::shared_ptr<request_router2>& router() const
	{
		return router_;
	}

protected:
	template<class Callback, typename... ArgDescriptors>
	void add_operation(verb method, path_spec&& path, Callback callback, ArgDescriptors... descriptors)
	{
		oas_.add_operation(method, path, callback, descriptors...);

		using ResultType = typename handler<Callback>::ResultType;

		// Note: the closure passed to add_route must be copyable.
		// This is why the handler is created as a shared_ptr, rather than a unique_ptr.
		// I would have preferred to use a unique_ptr, but then the handler would need to be
		// move-captured, thus making the lambda non-copyable and std::function would fail to create.
		auto h = std::make_shared<handler<Callback>>(this, callback, descriptors...);
		router_->add_route(
		    method, std::move(path), [handler = h, this](request&& req, url_view&& url, path_spec::param_map&& param) -> response_wrapper {
			    try
			    {
				    if constexpr (std::is_void_v<ResultType>)
				    {
					    handler->call(parameter_sources{ req, url, param });
					    auto res = empty_response::create(status::ok); //@@ status
					    res.version(req.version());
					    res.keep_alive(req.keep_alive());
					    return res;
				    }
				    else
				    {
					    auto res = make_response(handler->call(parameter_sources{ req, url, param }));
					    res.version(req.version());
					    res.keep_alive(req.keep_alive());
					    return res;
				    }
			    }
			    catch (const std::exception& e)
			    {
				    ZOO_LOG(err, "{}", e.what());
				    return make_error_response(e, req);
			    }
		    });
	}

	const boost::json::object& openapi_spec() const
	{
		return oas_.spec();
	}

private:
	template<typename T>
	response_wrapper make_response(T&& payload)
	{
		if constexpr (std::is_same_v<T, response_wrapper>)
		{
			return payload;
		}
		else if constexpr (is_http_response_v<std::decay_t<T>>)
		{
			return response_wrapper{ std::move(payload) };
		}
		else
		{
			return json_response::create(status::ok, std::move(payload)); //@@ status
		}
	}

	response make_error_response(const std::exception& e, const request& req)
	{
		auto       payload = ErrorType::create(e);
		const auto status  = payload.status();
		return json_response::create(status, std::move(payload));
	}

	std::shared_ptr<request_router2> router_;
	openapi<ErrorType>               oas_;
};

} // namespace spider
} // namespace zoo
