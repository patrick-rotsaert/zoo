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

class ZOO_SPIDER_API controller2
{
public:
	using p = parameters::p;

	class ZOO_SPIDER_API exception_handler_base
	{
	public:
		virtual ~exception_handler_base();

		virtual response handle(const std::exception& e, const request& req) = 0;
	};

private:
	// struct openapi_parameter_info final
	// {
	// 	parameters::descriptor descriptor;
	// 	std::type_index        type_id;
	// 	std::string            type_name; // Demangled type naam voor debugging
	// };

	// struct openapi_action_info final
	// {
	// 	verb                                method;
	// 	std::string                         openapi_path;
	// 	std::type_index                     return_type_id;
	// 	std::string                         return_type_name;
	// 	std::vector<openapi_parameter_info> parameters;
	// };

	// template<class Callback, typename... ArgDescriptors>
	// void store_route_info(verb method, std::string openapi_path, Callback callback, ArgDescriptors... descriptors)
	// {
	// 	using Handler    = handler<Callback>;
	// 	using ResultType = typename Handler::ResultType;
	// 	using ArgsTuple  = typename Handler::ArgsTuple;

	// 	auto info = openapi_action_info{ .method           = method,
	// 		                             .openapi_path     = std::move(openapi_path),
	// 		                             .return_type_id   = typeid(ResultType),
	// 		                             .return_type_name = demangled_type_name<ResultType>() };

	// 	// Verzamel de parameters
	// 	std::array<parameters::descriptor, sizeof...(descriptors)> parameter_descriptors = { parameters::descriptor{ descriptors }... };

	// 	// Helper om over de argumenttypen van de tuple te itereren
	// 	[&]<std::size_t... I>(std::index_sequence<I...>) {
	// 		(
	// 		    [&]() {
	// 			    using ArgType = typename std::tuple_element_t<I, ArgsTuple>;

	// 			    // We negeren de `const request&` en `const url_view&` argumenten,
	// 			    // aangezien deze niet direct OpenAPI-parameters zijn.
	// 			    if constexpr (!std::is_same_v<ArgType, request> && !std::is_same_v<ArgType, url_view>)
	// 			    {
	// 				    info.parameters.push_back({ parameter_descriptors[I], typeid(ArgType), demangled_type_name<ArgType>() });
	// 			    }
	// 		    }(),
	// 		    ...);
	// 	}(std::make_index_sequence<Handler::N>());

	// 	this->openapi_routes_.push_back(std::move(info));
	// }

	// In controller.hpp, na de controller klasse definitie

	// Hulpfunctie om een type_index naar een OpenAPI-schema te converteren.
	// Deze is zeer vereenvoudigd. Een echte implementatie zou introspectie via
	// Boost.Describe gebruiken om complexe schema's te genereren.
	// static boost::json::value schema_from_type_index(std::type_index type)
	// {
	// 	// Eenvoudige mapping van C++-typen naar OpenAPI-schema's
	// 	if (type == typeid(std::string) || type == typeid(std::optional<std::string>))
	// 	{
	// 		return { { "type", "string" } };
	// 	}
	// 	else if (type == typeid(std::uint64_t) || type == typeid(int) || type == typeid(std::optional<std::uint64_t>))
	// 	{
	// 		return { { "type", "integer" }, { "format", "int64" } };
	// 	}
	// 	else if (type == typeid(bool) || type == typeid(std::optional<bool>))
	// 	{
	// 		return { { "type", "boolean" } };
	// 	}
	// 	// TODO: Voeg ondersteuning toe voor het genereren van complexe schema's via Boost.Describe (zoals voor de 'customer' struct).
	// 	// Voor complexe objecten (Boost.Describe structs), zou je hier een JSON-schema moeten genereren
	// 	// op basis van de BOOST_DESCRIBE_STRUCT macro.
	// 	return { { "type", "object" } }; // Standaard voor complexe objecten
	// }

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
		if (this->exception_handler_)
		{
			return this->exception_handler_->handle(e, req);
		}
		else
		{
			ZOO_LOG(err, "{}", e.what());
			if (const auto status = boost::get_error_info<ex_status>(e))
			{
				return error_response_factory::create(req, *status);
			}
			else
			{
				return internal_server_error::create(req);
			}
		}
	}

	std::shared_ptr<request_router2>        router_;
	std::shared_ptr<exception_handler_base> exception_handler_;
	// std::vector<openapi_action_info>        openapi_routes_;
	openapi oas_;

public:
	virtual ~controller2();

	explicit controller2(std::shared_ptr<request_router2> router);

	const std::shared_ptr<request_router2>& router() const;

	void exception_handler(const std::shared_ptr<exception_handler_base>& value);

protected:
	template<class Callback, typename... ArgDescriptors>
	void add_operation(verb method, path_spec&& path, Callback callback, ArgDescriptors... descriptors)
	{
		using ResultType = typename handler<Callback>::ResultType;

		// this->store_route_info(method, path.to_string(), callback, descriptors...);
		this->oas_.add_operation(method, path, callback, descriptors...);

		// Note: the closure passed to add_route must be copyable.
		// This is why the handler is created as a shared_ptr, rather than a unique_ptr.
		// I would have preferred to use a unique_ptr, but then the handler would need to be
		// move-captured, thus making the lambda non-copyable and std::function would fail to create.
		auto h = std::make_shared<handler<Callback>>(this, callback, descriptors...);
		this->router_->add_route(
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
				    return make_error_response(e, req);
			    }
		    });
	}

	const boost::json::object& openapi_spec() const
	{
		return oas_.spec();
	}

#if 0
	// Hulpfunctie om een type_index naar een OpenAPI-parameter te converteren.
	boost::json::value to_openapi_spec() const
	{
		using namespace boost::json;

		object spec;
		spec["openapi"] = "3.0.0";
		spec["info"]    = { { "title", "Generated API" }, { "version", "1.0.0" } };

		// Verzamel alle paden en operaties
		object paths_object;

		for (const auto& route : this->openapi_routes_)
		{
			auto& path_item = paths_object[route.openapi_path].emplace_object();
			// auto& path_item = paths_object[route.openapi_path].as_object();

			// Converteer verb-enum naar lowercase string ("get", "post", etc.)
			auto        method_sv = to_string(route.method);
			std::string method_str{ method_sv.begin(), method_sv.end() };
			boost::algorithm::to_lower(method_str);

			object operation;

			// 1. Parameters (Input)
			array parameters;
			for (const auto& param_info : route.parameters)
			{
				object param;
				std::visit(
				    [&](auto&& p) {
					    using P = std::decay_t<decltype(p)>;
					    if constexpr (std::is_same_v<P, p::path>)
					    {
						    param["name"]     = p.name;
						    param["in"]       = "path";
						    param["required"] = true; // Padparameters zijn altijd verplicht
						    param["schema"]   = schema_from_type_index(param_info.type_id);
					    }
					    else if constexpr (std::is_same_v<P, p::query>)
					    {
						    param["name"] = p.name;
						    param["in"]   = "query";
						    // Bepaal 'required': Dit is complexer, maar we nemen aan dat
						    // std::optional of boost::optional 'niet vereist' betekent.
						    param["required"] = !(param_info.type_name.find("optional") != std::string::npos);
						    param["schema"]   = schema_from_type_index(param_info.type_id);
					    }
					    else if constexpr (std::is_same_v<P, p::header>)
					    {
						    param["name"]     = p.name;
						    param["in"]       = "header";
						    param["required"] = !(param_info.type_name.find("optional") != std::string::npos);
						    param["schema"]   = schema_from_type_index(param_info.type_id);
					    }
					    else if constexpr (std::is_same_v<P, p::json>)
					    {
						    // JSON-parameters zijn Request Body, geen Parameters object
						    // Dit wordt later behandeld
					    }
				    },
				    param_info.descriptor);

				if (!param.empty() && param["in"] != "body") // Voeg toe tenzij het een JSON body is
				{
					parameters.emplace_back(std::move(param));
				}
			}
			if (!parameters.empty())
			{
				operation["parameters"] = std::move(parameters);
			}

			// 2. Request Body (voor POST/PUT met JSON)
			// Zoek naar de p::json descriptor
			for (const auto& param_info : route.parameters)
			{
				if (std::holds_alternative<p::json>(param_info.descriptor))
				{
					operation["requestBody"] = {
						{ "required", true },
						{ "content", { { "application/json", { { "schema", schema_from_type_index(param_info.type_id) } } } } }
					};
					break;
				}
			}

			// 3. Responses (Output)
			object responses;
			responses["200"]       = { { "description", fmt::format("Succesvolle respons met object van type {}", route.return_type_name) },
				                       { "content",
				                         { { "application/json", { { "schema", schema_from_type_index(route.return_type_id) } } } } } };
			operation["responses"] = std::move(responses);

			// Voeg de operatie toe aan het paditem
			path_item[method_str] = std::move(operation);
		}

		spec["paths"] = std::move(paths_object);

		// Hier zou je ook een "components" sectie met herbruikbare schema's toevoegen.

		return spec;
	}
#endif
};

} // namespace spider
} // namespace zoo
