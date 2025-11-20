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
#include "zoo/spider/rest/operation.h"
#include "zoo/spider/rest/handler.hpp"
#include "zoo/spider/rest/openapi.hpp"
#include "zoo/spider/rest/parameters.h"
#include "zoo/spider/rest/router.h"
#include "zoo/spider/concepts.hpp"
#include "zoo/spider/response_wrapper.hpp"
#include "zoo/spider/json_response.h"
#include "zoo/spider/binary_response.h"
#include "zoo/spider/empty_response.h"
#include "zoo/spider/exception.h"

#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/misc/demangled_type_name.hpp"
#include "zoo/common/misc/is_variant.hpp"

#include <boost/json.hpp>

#include <type_traits>

namespace zoo {
namespace spider {

template<IsValidErrorType DefaultErrorType>
class rest_controller
{
public:
	using p            = parameters::p;
	using openapi_type = openapi<DefaultErrorType>;

	explicit rest_controller(std::shared_ptr<rest_router> router, openapi_settings settings)
	    : router_{ std::move(router) }
	    , oas_{ std::move(settings) }
	{
	}

	virtual ~rest_controller() = default;

	const std::shared_ptr<rest_router>& router() const
	{
		return router_;
	}

	const openapi_type& oas() const
	{
		return oas_;
	}

protected:
	template<class Callback, typename... ArgDescriptors>
	void add_operation(rest_operation op, Callback callback, ArgDescriptors... descriptors)
	{
		oas_.add_operation(op, callback, descriptors...);

		using ResultType = typename handler<Callback>::ResultType;

		// Note: the closure passed to add_route must be copyable.
		// This is why the handler is created as a shared_ptr, rather than a unique_ptr.
		// I would have preferred to use a unique_ptr, but then the handler would need to be
		// move-captured, thus making the lambda non-copyable and std::function would fail to create.
		const auto method = op.method;
		auto       h      = std::make_shared<handler<Callback>>(this, callback, descriptors...);
		router_->add_route(std::move(op),
		                   [method, handler = h, this](request&& req, url_view&& url, path_spec::param_map&& param) -> response_wrapper {
			                   try
			                   {
				                   if constexpr (std::is_void_v<ResultType>)
				                   {
					                   handler->call(parameter_sources{ req, url, param });
					                   auto res = empty_response::create(status::no_content);
					                   res.version(req.version());
					                   res.keep_alive(req.keep_alive());
					                   return res;
				                   }
				                   else
				                   {
					                   auto res = make_response(handler->call(parameter_sources{ req, url, param }),
					                                            status_utility::success_status_for_method(method));
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

private:
	template<typename T>
	static response_wrapper make_response(T&& payload, http::status status)
	{
		if constexpr (std::is_same_v<T, response_wrapper>)
		{
			return payload;
		}
		else if constexpr (is_http_response_v<std::decay_t<T>>)
		{
			return response_wrapper{ std::move(payload) };
		}
		else if constexpr (is_variant_v<T>)
		{
			return std::visit([status](auto&& arg) -> response_wrapper { return make_response(std::move(arg), status); },
			                  std::move(payload));
		}
		else if constexpr (IsStatusResult<T>)
		{
			return make_response(std::move(payload.result), T::STATUS);
		}
		else if constexpr (IsBinaryContentContainer<T>)
		{
			return binary_response::create(status, payload.content_type(), payload.content());
		}
		else
		{
			return json_response::create(status, std::move(payload));
		}
	}

	static response make_error_response(const std::exception& e, const request& req)
	{
		auto       payload = DefaultErrorType::create(e);
		const auto status  = payload.status();
		return json_response::create(req, status, std::move(payload));
	}

	std::shared_ptr<rest_router> router_;
	openapi_type                 oas_;
};

} // namespace spider
} // namespace zoo
