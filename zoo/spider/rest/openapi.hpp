//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/rest/handler.hpp"
#include "zoo/spider/rest/operation.h"
#include "zoo/spider/rest/concepts.hpp"
#include "zoo/spider/rest/status_utility.hpp"
#include "zoo/spider/rest/annotation.hpp"
#include "zoo/spider/rest/security.h"
#include "zoo/common/misc/is_optional.hpp"
#include "zoo/common/misc/is_vector.hpp"
#include "zoo/common/misc/is_variant.hpp"
#include "zoo/common/logging/logging.h"

#include <boost/json.hpp>
#include <boost/json/conversion.hpp>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <tuple>
#include <array>
#include <limits>
#include <map>
#include <unordered_map>

namespace zoo {
namespace spider {

struct openapi_settings
{
	std::string_view strip_ns;
	std::string_view info_title;
	std::string_view info_version;
};

template<IsValidErrorType DefaultErrorType>
class openapi final
{
public:
	explicit openapi(openapi_settings settings)
	    : settings_{ std::move(settings) }
	    , spec_{}
	    , have_global_security_{}
	{
		using namespace boost::json;

		spec_["openapi"] = "3.0.0";
		spec_["info"]    = { { "title", settings_.info_title }, { "version", settings_.info_version } };

		ensure_object(spec_["paths"]); // make sure paths appears before components
	}

	template<class Callback, typename... ArgDescriptors>
	void add_operation(rest_operation op, Callback, ArgDescriptors... descriptors)
	{
		using namespace boost::json;

		using Handler    = handler<Callback>;
		using ResultType = typename Handler::ResultType;
		using ArgsTuple  = typename Handler::ArgsTuple;

		std::array<parameters::descriptor, sizeof...(descriptors)> parameter_descriptors = { parameters::descriptor{ descriptors }... };

		object operation;

		if (!op.operation_id.empty())
		{
			operation["operationId"] = op.operation_id;
		}
		if (!op.summary.empty())
		{
			operation["summary"] = op.summary;
		}
		if (op.sec)
		{
			operation["security"] = security_array(op.sec.value());
		}

		{
			array parameters;

			[&]<std::size_t... I>(std::index_sequence<I...>) {
				(
				    [&]() {
					    using ArgType          = typename std::tuple_element_t<I, ArgsTuple>;
					    const auto& descriptor = parameter_descriptors[I];

					    boost::json::object param;

					    std::visit(
					        [&](auto&& arg) {
						        using p = parameters::p;
						        using P = std::decay_t<decltype(arg)>;
						        if constexpr (std::is_same_v<P, p::path>)
						        {
							        param["name"]     = arg.name;
							        param["in"]       = "path";
							        param["required"] = true;
							        param["schema"]   = value_schema<ArgType>();
						        }
						        else if constexpr (std::is_same_v<P, p::query>)
						        {
							        param["name"]     = arg.name;
							        param["in"]       = "query";
							        param["required"] = !is_optional_v<ArgType>;
							        param["schema"]   = value_schema<ArgType>();
						        }
						        else if constexpr (std::is_same_v<P, p::header>)
						        {
							        param["name"]     = arg.name;
							        param["in"]       = "header";
							        param["required"] = !is_optional_v<ArgType>;
							        param["schema"]   = value_schema<ArgType>();
						        }
						        else if constexpr (std::is_same_v<P, p::json>)
						        {
							        operation["requestBody"] = {
								        { "required", true },
								        { "content", { { "application/json", { { "schema", value_schema<ArgType>() } } } } }
							        };
						        }
						        else if constexpr (std::is_same_v<P, p::request> || std::is_same_v<P, p::url> || std::is_same_v<P, p::auth>)
						        {
							        return;
						        }
						        else
						        {
							        static_assert(false, "non-exhaustive visitor!");
						        }

						        if constexpr (std::is_base_of_v<parameters::named_parameter, P>)
						        {
							        if (!arg.description.empty())
							        {
								        param["description"] = arg.description;
							        }
						        }
					        },
					        descriptor);

					    if (!param.empty())
					    {
						    parameters.emplace_back(std::move(param));
					    }
				    }(),
				    ...);
			}(std::make_index_sequence<Handler::N>());

			if (!parameters.empty())
			{
				operation["parameters"] = std::move(parameters);
			}
		}

		{
			object responses;

			add_response<ResultType>(responses, status_utility::success_status_for_method(op.method));

			if ((op.sec && !is_empty_security(op.sec.value())) || have_global_security_)
			{
				const auto unauthorized = std::to_string(static_cast<int>(status::unauthorized));
				if (!responses.contains(unauthorized))
				{
					responses[unauthorized] = { { "$ref", get_unauthorized_response_ref() } };
				}
			}

			responses["default"] = { { "$ref", get_default_response_ref() } };

			operation["responses"] = std::move(responses);
		}

		auto& paths_object                  = ensure_object(spec_["paths"]);
		auto& path_object                   = ensure_object(paths_object["/" + op.path.to_string()]);
		path_object[method_name(op.method)] = std::move(operation);
	}

	void set_global_security(const security& sec)
	{
		spec_["security"]     = security_array(sec);
		have_global_security_ = !is_empty_security(sec);
	}

	const boost::json::object& spec() const
	{
		return spec_;
	}

private:
	static bool is_empty_security(const security& sec)
	{
		for (const auto& map : sec)
		{
			if (!map.empty())
			{
				return false;
			}
		}
		return true;
	}

	static std::string method_name(verb method)
	{
		auto        sv = to_string(method);
		std::string name{ sv.begin(), sv.end() };
		boost::algorithm::to_lower(name);
		return name;
	}

	template<typename T>
	static std::string_view get_content_type()
	{
		if constexpr (IsStatusResult<T>)
		{
			return get_content_type<typename T::value_type>();
		}
		else if constexpr (IsBinaryContentContainer<T> || IsStringContentContainer<T>)
		{
			return T::CONTENT_TYPE;
		}
		else if constexpr (ConvertibleToBoostJson<T>)
		{
			return "application/json";
		}
		else
		{
			return {};
		}
	}

	static boost::json::object& ensure_object(boost::json::value& v)
	{
		return v.is_object() ? v.as_object() : v.emplace_object();
	}

	template<typename T>
	void add_response(boost::json::object& responses, http::status status)
	{
		if constexpr (std::is_void_v<T>)
		{
			responses[std::to_string(static_cast<int>(http::status::no_content))] = { { "description",
				                                                                        http::obsolete_reason(http::status::no_content) } };
		}
		else if constexpr (IsStatusResult<T>)
		{
			return add_response<typename T::value_type>(responses, T::STATUS);
		}
		else if constexpr (IsBinaryContentContainer<T> || IsStringContentContainer<T>)
		{
			responses[std::to_string(static_cast<int>(status))] = {
				{ "description", http::obsolete_reason(status) },
				{ "content", { { get_content_type<T>(), { { "schema", value_schema<T>() } } } } }
			};
		}
		else if constexpr (is_variant_v<T>)
		{
			add_variant_response<T>(responses, status);
		}
		else if constexpr (ConvertibleToBoostJson<T>)
		{
			responses[std::to_string(static_cast<int>(status))] = {
				{ "description", http::obsolete_reason(status) },
				{ "content", { { get_content_type<T>(), { { "schema", value_schema<T>() } } } } }
			};
		}
		else
		{
			zlog(warn, "Unsupported response type {}", demangled_type_name<T>());
		}
	}

	template<typename T>
	void add_variant_response(boost::json::object& responses, http::status status)
	{
		std::map<http::status, std::unordered_map<std::string_view, std::vector<boost::json::object>>> map;

		[&]<std::size_t... I>(std::index_sequence<I...>) {
			(
			    [&]() {
				    using V = std::variant_alternative_t<I, T>;
				    if constexpr (IsStatusResult<V>)
				    {
					    map[V::STATUS][get_content_type<V>()].push_back(value_schema<typename V::value_type>());
				    }
				    else if constexpr (IsBinaryContentContainer<V> || IsStringContentContainer<V> || ConvertibleToBoostJson<V>)
				    {
					    map[status][get_content_type<V>()].push_back(value_schema<V>());
				    }
				    else
				    {
					    zlog(warn, "Unsupported response variant type {}", demangled_type_name<V>());
				    }
			    }(),
			    ...);
		}(std::make_index_sequence<std::variant_size_v<T>>());

		for (const auto& status_pair : map)
		{
			const auto          status      = status_pair.first;
			const auto&         content_map = status_pair.second;
			boost::json::object response;
			response["description"] = http::obsolete_reason(status);
			boost::json::object content;
			for (const auto& content_pair : content_map)
			{
				const auto&         content_type = content_pair.first;
				auto&               schemas      = content_pair.second;
				boost::json::object type_content;
				if (schemas.size() > 1u)
				{
					boost::json::array oneof;
					for (auto& schema : schemas)
					{
						oneof.emplace_back(std::move(schema));
					}
					type_content["schema"] = { { "oneOf", std::move(oneof) } };
				}
				else
				{
					type_content["schema"] = std::move(schemas.front());
				}
				content[content_type] = std::move(type_content);
			}
			response["content"]                                 = std::move(content);
			responses[std::to_string(static_cast<int>(status))] = std::move(response);
		}
	}

	template<typename T>
	std::enable_if_t<is_optional_v<T>, boost::json::object> value_schema()
	{
		return value_schema<typename T::value_type>();
	}

	template<typename T>
	std::enable_if_t<!is_optional_v<T>, boost::json::object> value_schema()
	{
		boost::json::object schema;

		if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>)
		{
			schema["type"] = "string";
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			schema["type"] = "boolean";
		}
		else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t> || std::is_same_v<T, uint16_t> ||
		                   std::is_same_v<T, int16_t>)
		{
			schema["type"]    = "integer";
			schema["format"]  = "int32";
			schema["minimum"] = std::numeric_limits<T>::min();
			schema["maximum"] = std::numeric_limits<T>::max();
		}
		else if constexpr (std::is_same_v<T, uint32_t>)
		{
			schema["type"]    = "integer";
			schema["format"]  = "int64";
			schema["minimum"] = std::numeric_limits<T>::min();
			schema["maximum"] = std::numeric_limits<T>::max();
		}
		else if constexpr (std::is_same_v<T, int32_t>)
		{
			schema["type"]   = "integer";
			schema["format"] = "int32";
		}
		else if constexpr (std::is_same_v<T, uint64_t>)
		{
			schema["type"]    = "integer";
			schema["format"]  = "int64";
			schema["minimum"] = 0;
		}
		else if constexpr (std::is_same_v<T, int64_t>)
		{
			schema["type"]   = "integer";
			schema["format"] = "int64";
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			schema["type"]   = "number";
			schema["format"] = "float";
		}
		else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, long double>)
		{
			schema["type"]   = "number";
			schema["format"] = "double";
		}
		else if constexpr (std::is_same_v<T, boost::gregorian::date> || std::is_same_v<T, conversion::date>)
		{
			schema["type"]   = "string";
			schema["format"] = "date";
		}
		else if constexpr (std::is_same_v<T, boost::posix_time::time_duration> || std::is_same_v<T, conversion::time_of_day>)
		{
			schema["type"]   = "string";
			schema["format"] = "time";
		}
		else if constexpr (std::is_same_v<T, boost::posix_time::ptime> || std::is_same_v<T, conversion::time_point>)
		{
			schema["type"]   = "string";
			schema["format"] = "date-time";
		}
		else if constexpr (std::is_same_v<T, boost::uuids::uuid>)
		{
			schema["type"]   = "string";
			schema["format"] = "uuid";
		}
		else if constexpr (std::is_enum_v<T> && boost::json::is_described_enum<T>::value)
		{
			schema["$ref"] = add_components_schema<T>();
		}
		else if constexpr (is_vector_v<T>)
		{
			schema["type"]  = "array";
			schema["items"] = value_schema<typename T::value_type>();
		}
		else if constexpr (boost::json::is_described_class<T>::value)
		{
			schema["$ref"] = add_components_schema<T>();
		}
		else if constexpr (IsBinaryContentContainer<T>)
		{
			schema["type"]   = "string";
			schema["format"] = "binary";
		}
		else if constexpr (IsStringContentContainer<T>)
		{
			schema["type"] = "string";
		}
		else
		{
			zlog(warn, "Unsupported value type {}", demangled_type_name<T>());
		}

		return schema;
	}

	template<typename T>
	std::enable_if_t<std::is_enum_v<T> && boost::json::is_described_enum<T>::value, std::string> add_components_schema()
	{
		const auto type_name = get_type_name<T>();

		if (!components_schema_exists(type_name))
		{
			boost::json::object schema;

			schema["type"] = "string";
			auto& e        = schema["enum"].emplace_array();
			boost::mp11::mp_for_each<boost::describe::describe_enumerators<T>>([&](auto D) { e.emplace_back(D.name); });

			add_components_schema(type_name, std::move(schema));
		}

		return components_schema_ref(type_name);
	}

	template<typename T>
	std::enable_if_t<boost::json::is_described_class<T>::value && boost::describe::has_describe_members<T>::value, std::string>
	add_components_schema()
	{
		const auto type_name = get_type_name<T>();

		if (!components_schema_exists(type_name))
		{
			boost::json::object schema;

			schema["type"] = "object";

			boost::json::object properties;
			boost::json::array  required;

			boost::mp11::mp_for_each<boost::describe::describe_members<T, boost::describe::mod_any_access>>([&](auto D) {
				T* helper{};
				using U = std::decay_t<decltype(helper->*D.pointer)>;
				if constexpr (!is_optional_v<U>)
				{
					required.emplace_back(D.name);
				}
				properties[D.name]      = value_schema<U>();
				const auto& description = annotation_v<decltype(D)>;
				if (!description.empty())
				{
					properties[D.name].as_object()["description"] = description;
				}
			});

			if (!properties.empty())
			{
				schema["properties"] = std::move(properties);
			}

			if (!required.empty())
			{
				schema["required"] = std::move(required);
			}

			if constexpr (HasStaticExampleMethod<T>)
			{
				schema["example"] = boost::json::value_from(T::example());
			}

			add_components_schema(type_name, std::move(schema));
		}

		return components_schema_ref(type_name);
	}

	void add_components_schema(std::string_view type_name, boost::json::object schema)
	{
		auto& components   = ensure_object(spec_["components"]);
		auto& schemas      = ensure_object(components["schemas"]);
		schemas[type_name] = std::move(schema);
	}

	bool components_schema_exists(std::string_view type_name) const
	{
		if (spec_.contains("components"))
		{
			const auto& components = spec_.at("components").as_object();
			if (components.contains("schemas"))
			{
				const auto& schemas = components.at("schemas").as_object();
				return schemas.contains(type_name);
			}
		}
		return false;
	}

	static std::string components_schema_ref(std::string_view type_name)
	{
		return fmt::format("#/components/schemas/{}", type_name);
	}

	std::string get_unauthorized_response_ref()
	{
		const auto type_name = "unauthorized";
		if (components_response_exists(type_name))
		{
			return components_response_ref(type_name);
		}
		else
		{
			boost::json::object response = { { "description", "Authorization information is missing or invalid." },
				                             { "content",
				                               { { "application/json", { { "schema", value_schema<DefaultErrorType>() } } } } } };
			return add_components_response(type_name, std::move(response));
		}
	}

	std::string get_default_response_ref()
	{
		const auto type_name = "default";
		if (components_response_exists(type_name))
		{
			return components_response_ref(type_name);
		}
		else
		{
			boost::json::object response = { { "description", "An unexpected error occurred." },
				                             { "content",
				                               { { "application/json", { { "schema", value_schema<DefaultErrorType>() } } } } } };
			return add_components_response(type_name, std::move(response));
		}
	}

	std::string add_components_response(std::string_view type_name, boost::json::object response)
	{
		auto& components     = ensure_object(spec_["components"]);
		auto& responses      = ensure_object(components["responses"]);
		responses[type_name] = std::move(response);
		return components_response_ref(type_name);
	}

	bool components_response_exists(std::string_view type_name) const
	{
		if (spec_.contains("components"))
		{
			const auto& components = spec_.at("components").as_object();
			if (components.contains("responses"))
			{
				const auto& responses = components.at("responses").as_object();
				return responses.contains(type_name);
			}
		}
		return false;
	}

	static std::string components_response_ref(std::string_view type_name)
	{
		return fmt::format("#/components/responses/{}", type_name);
	}

	template<typename T>
	std::string get_type_name()
	{
		if constexpr (HasStaticTypeNameMethod<T>)
		{
			return std::string{ T::type_name() };
		}
		else
		{
			auto type_name = demangled_type_name<T>();
			if (!settings_.strip_ns.empty() && std::string_view{ type_name }.starts_with(settings_.strip_ns))
			{
				type_name = type_name.substr(settings_.strip_ns.length());
			}
			boost::algorithm::replace_all(type_name, "::", "_");
			boost::algorithm::replace_all(type_name, "<", "_");
			boost::algorithm::replace_all(type_name, ">", "_");
			return type_name;
		}
	}

	boost::json::array security_array(const security& sec)
	{
		boost::json::array a;
		for (const auto& map : sec)
		{
			boost::json::object o;
			for (const auto& pair : map)
			{
				const auto& scheme = *pair.first;
				const auto& scopes = pair.second;
				add_components_security_scheme(scheme);
				o[scheme.scheme_name()] = boost::json::value_from(scopes);
			}
			a.emplace_back(std::move(o));
		}
		return a;
	}

	void add_components_security_scheme(const isecurityscheme& scheme)
	{
		auto& components = ensure_object(spec_["components"]);
		auto& schemes    = ensure_object(components["securitySchemes"]);
		if (!schemes.contains(scheme.scheme_name()))
		{
			schemes[scheme.scheme_name()] = scheme.scheme();
		}
	}

	openapi_settings    settings_;
	boost::json::object spec_;
	bool                have_global_security_;
};

} // namespace spider
} // namespace zoo
