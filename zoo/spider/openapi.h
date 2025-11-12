//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/handler.hpp"
#include "zoo/common/misc/is_optional.hpp"
#include "zoo/common/misc/is_vector.hpp"
#include "zoo/common/logging/logging.h"

#include <boost/json.hpp>
#include <boost/json/conversion.hpp>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>

#include <tuple>
#include <array>
#include <limits>

namespace zoo {
namespace spider {

class openapi final
{
public:
	explicit openapi();

	template<class Callback, typename... ArgDescriptors>
	void add_operation(verb method, const path_spec& path, Callback callback, ArgDescriptors... descriptors)
	{
		using namespace boost::json;

		using Handler = handler<Callback>;
		// using ResultType = typename Handler::ResultType;
		using ArgsTuple = typename Handler::ArgsTuple;

		std::array<parameters::descriptor, sizeof...(descriptors)> parameter_descriptors = { parameters::descriptor{ descriptors }... };

		auto& paths_object = ensure_object(spec_["paths"]);
		auto& path_object  = ensure_object(paths_object[path.to_string()]);
		auto& operation    = ensure_object(path_object[method_name(method)]);

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
						        operation["requestBody"] = { { "required", true },
							                                 { "content",
							                                   { { "application/json", { { "schema", value_schema<ArgType>() } } } } } };
					        }
					        else if constexpr (std::is_same_v<P, p::request>)
					        {
						        return;
					        }
					        else if constexpr (std::is_same_v<P, p::url>)
					        {
						        return;
					        }
					        else
					        {
						        static_assert(false, "non-exhaustive visitor!");
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

	const boost::json::object& spec() const;

private:
	static std::string method_name(verb method); //@@

	static boost::json::object& ensure_object(boost::json::value& v)
	{
		return v.is_object() ? v.as_object() : v.emplace_object();
	}

	static boost::json::array& ensure_array(boost::json::value& v)
	{
		return v.is_array() ? v.as_array() : v.emplace_array();
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

		// schema["__type__"] = demangled_type_name<T>(); //@@

		if constexpr (std::is_same_v<T, std::string>)
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
		// else
		// {
		// 	schema["__unknown__"] = true; //@@
		// }

		return schema;
	}

	template<typename T>
	std::enable_if_t<std::is_enum_v<T> && boost::json::is_described_enum<T>::value, std::string> add_components_schema()
	{
		const auto type_name = get_type_name<T>();

		auto& schema = upsert_component_schema(type_name);

		schema["type"] = "string";
		auto& e        = schema["enum"].emplace_array();
		boost::mp11::mp_for_each<boost::describe::describe_enumerators<T>>([&](auto D) { e.emplace_back(D.name); });

		return component_schema_ref(type_name);
	}

	template<typename T>
	std::enable_if_t<boost::json::is_described_class<T>::value && boost::describe::has_describe_members<T>::value, std::string>
	add_components_schema()
	{
		const auto type_name = get_type_name<T>();

		auto& schema = upsert_component_schema(type_name);

		schema["type"] = "object";

		boost::json::array  required;
		boost::json::object properties;

		boost::mp11::mp_for_each<boost::describe::describe_members<T, boost::describe::mod_any_access>>([&](auto D) {
			T* helper{};
			using U = std::decay_t<decltype(helper->*D.pointer)>;
			if constexpr (!is_optional_v<U>)
			{
				required.emplace_back(D.name);
			}
			properties[D.name] = value_schema<U>();
		});

		if (!required.empty())
		{
			schema["required"] = std::move(required);
		}

		if (!properties.empty())
		{
			schema["properties"] = std::move(properties);
		}

		return component_schema_ref(type_name);
	}

	boost::json::object& upsert_component_schema(std::string_view type_name)
	{
		auto& components = ensure_object(spec_["components"]);
		auto& schemas    = ensure_object(components["schemas"]);
		return schemas[type_name].emplace_object();
	}

	static std::string component_schema_ref(std::string_view type_name)
	{
		return fmt::format("#/components/schemas/{}", type_name);
	}

	template<typename T>
	std::string get_type_name()
	{
		return demangled_type_name<T>(); //@@ FIXME: strip leading namespaces?
	}

	boost::json::object spec_;
};

} // namespace spider
} // namespace zoo
