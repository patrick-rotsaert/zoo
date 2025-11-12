//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/handler.hpp"
#include "zoo/common/misc/is_optional.hpp"

#include <boost/json.hpp>
#include <boost/json/conversion.hpp>
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
						        param["schema"]   = parameter_schema<ArgType>();
					        }
					        else if constexpr (std::is_same_v<P, p::query>)
					        {
						        param["name"]     = arg.name;
						        param["in"]       = "query";
						        param["required"] = !is_optional_v<ArgType>;
						        param["schema"]   = parameter_schema<ArgType>();
					        }
					        else if constexpr (std::is_same_v<P, p::header>)
					        {
						        param["name"]     = arg.name;
						        param["in"]       = "header";
						        param["required"] = !is_optional_v<ArgType>;
						        param["schema"]   = parameter_schema<ArgType>();
					        }
					        else if constexpr (std::is_same_v<P, p::json>)
					        {
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

	template<typename ArgType>
	std::enable_if_t<is_optional_v<ArgType>, boost::json::object> parameter_schema()
	{
		return parameter_schema<typename ArgType::value_type>();
	}

	template<typename ArgType>
	std::enable_if_t<!is_optional_v<ArgType>, boost::json::object> parameter_schema()
	{
		boost::json::object schema;

		if constexpr (std::is_same_v<ArgType, std::string>)
		{
			schema["type"] = "string";
		}
		else if constexpr (std::is_same_v<ArgType, bool>)
		{
			schema["type"] = "boolean";
		}
		else if constexpr (std::is_same_v<ArgType, uint8_t> || std::is_same_v<ArgType, int8_t> || std::is_same_v<ArgType, uint16_t> ||
		                   std::is_same_v<ArgType, int16_t>)
		{
			schema["type"]    = "integer";
			schema["format"]  = "int32";
			schema["minimum"] = std::numeric_limits<ArgType>::min();
			schema["maximum"] = std::numeric_limits<ArgType>::max();
		}
		else if constexpr (std::is_same_v<ArgType, uint32_t>)
		{
			schema["type"]    = "integer";
			schema["format"]  = "int64";
			schema["minimum"] = std::numeric_limits<ArgType>::min();
			schema["maximum"] = std::numeric_limits<ArgType>::max();
		}
		else if constexpr (std::is_same_v<ArgType, int32_t>)
		{
			schema["type"]   = "integer";
			schema["format"] = "int32";
		}
		else if constexpr (std::is_same_v<ArgType, uint64_t>)
		{
			schema["type"]    = "integer";
			schema["format"]  = "int64";
			schema["minimum"] = 0;
		}
		else if constexpr (std::is_same_v<ArgType, int64_t>)
		{
			schema["type"]   = "integer";
			schema["format"] = "int64";
		}
		else if constexpr (std::is_same_v<ArgType, float>)
		{
			schema["type"]   = "number";
			schema["format"] = "float";
		}
		else if constexpr (std::is_same_v<ArgType, double> || std::is_same_v<ArgType, long double>)
		{
			schema["type"]   = "number";
			schema["format"] = "double";
		}
		else if constexpr (std::is_same_v<ArgType, boost::gregorian::date> || std::is_same_v<ArgType, conversion::date>)
		{
			schema["type"]   = "string";
			schema["format"] = "date";
		}
		else if constexpr (std::is_same_v<ArgType, boost::posix_time::time_duration> || std::is_same_v<ArgType, conversion::time_of_day>)
		{
			schema["type"]   = "string";
			schema["format"] = "time";
		}
		else if constexpr (std::is_same_v<ArgType, boost::posix_time::ptime> || std::is_same_v<ArgType, conversion::time_point>)
		{
			schema["type"]   = "string";
			schema["format"] = "date-time";
		}
		else if constexpr (std::is_enum_v<ArgType> && boost::json::is_described_enum<ArgType>::value)
		{
			schema["type"] = "string";
			auto& e        = schema["enum"].emplace_array();
			boost::mp11::mp_for_each<boost::describe::describe_enumerators<ArgType>>([&](auto D) { e.emplace_back(D.name); });
		}

		return schema;
	}

	boost::json::object spec_;
};

} // namespace spider
} // namespace zoo
