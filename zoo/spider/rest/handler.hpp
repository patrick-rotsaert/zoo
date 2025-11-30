//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/rest/parameters.h"
#include "zoo/spider/rest/conversions.h"
#include "zoo/spider/concepts.hpp"
#include "zoo/spider/exception.h"
#include "zoo/common/misc/throw_exception.h"
#include "zoo/common/misc/demangled_type_name.hpp"
#include "zoo/common/misc/is_optional.hpp"
#include "zoo/common/logging/logging.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <type_traits>
#include <string>
#include <variant>
#include <tuple>
#include <array>
#include <functional>
#include <stdexcept>

namespace zoo {
namespace spider {

template<typename Controller, typename...>
class handler final
{
};

template<typename Controller, typename Result, typename... Args>
class handler<Result (Controller::*)(Args...)> final
{
public:
	using Method     = Result (Controller::*)(Args...);
	using ArgsTuple  = std::tuple<typename std::remove_const_t<typename std::remove_reference_t<Args>>...>;
	using ResultType = typename std::remove_const_t<typename std::remove_reference_t<Result>>;
	using Callback   = std::function<ResultType(Args...)>;

	static constexpr size_t N = sizeof...(Args);

	template<typename Owner, typename... Descriptors>
	handler(Owner* owner, Method method, Descriptors... descriptors)
	    : callback_{}
	    , descriptors_{ { parameters::descriptor{ descriptors }... } }
	{
		static_assert(sizeof...(descriptors) == sizeof...(Args),
		              "The number of descriptors must be equal to the number of arguments of the callback function");

		auto controller = dynamic_cast<Controller*>(owner);
		if (controller == nullptr)
		{
			ZOO_THROW_EXCEPTION(std::invalid_argument{ "Callback must be provided by a class derived from controller" });
		}

		callback_ = [controller, method](Args... args) { return (controller->*method)(args...); };

		validate_descriptors(std::make_index_sequence<N>());
	}

	ResultType call(const parameter_sources& sources)
	{
		try
		{
			return invoke(collect_arguments(sources, std::make_index_sequence<N>()));
		}
		catch (const argument_error& e)
		{
			// The type `argument_error` is private, so we're sure that the throw site is in the arguments collection.
			// Set the appropriate http status in the exception.
			e << ex_status{ status::bad_request };
			throw;
		}
	}

private:
	using p = parameters::p;

	class ZOO_SPIDER_API argument_error : public exception_base
	{
	public:
		explicit argument_error(const std::string& m)
		    : exception_base{ m }
		{
		}
	};

	template<std::size_t... I>
	void validate_descriptors(std::index_sequence<I...>)
	{
		((validate_descriptor<typename std::tuple_element_t<I, ArgsTuple>>(descriptors_[I])), ...);
	}

	template<typename T>
	void validate_descriptor(const parameters::descriptor& descriptor)
	{
		auto ok = false;
		std::visit(
		    [&](auto&& p) {
			    using P = std::decay_t<decltype(p)>;
			    if constexpr (std::is_same_v<P, p::path> || std::is_same_v<P, p::query> || std::is_same_v<P, p::header>)
			    {
				    ok = true;
			    }
			    else if constexpr (std::is_same_v<P, p::auth>)
			    {
				    ok = std::is_base_of_v<auth_data_base, T>;
			    }
			    else if constexpr (std::is_same_v<P, p::json>)
			    {
				    ok = ConvertibleFromBoostJson<T>;
			    }
			    else if constexpr (std::is_same_v<P, p::request>)
			    {
				    ok = std::is_same_v<T, request>;
			    }
			    else if constexpr (std::is_same_v<P, p::url>)
			    {
				    ok = std::is_same_v<T, url_view>;
			    }
			    else
			    {
				    static_assert(false, "non-exhaustive visitor!");
			    }
		    },
		    descriptor);

		if (!ok)
		{
			const auto descriptor_type_name = std::visit([](auto&& arg) { return demangled_type_name(arg); }, descriptor);
			ZOO_THROW_EXCEPTION(
			    std::logic_error{ fmt::format("Descriptor type `{}` is incompatible with argument type `{}` for method `{}`",
			                                  descriptor_type_name,
			                                  demangled_type_name<T>(),
			                                  demangled_type_name<Method>()) });
		}
	}

	template<typename ArgsTuple>
	ResultType invoke(ArgsTuple&& args)
	{
		return std::apply(callback_, std::forward<ArgsTuple>(args));
	}

	template<std::size_t... I>
	ArgsTuple collect_arguments(const parameter_sources& sources, std::index_sequence<I...>)
	{
		return std::make_tuple(collect_argument<typename std::tuple_element_t<I, ArgsTuple>>(
		    sources, descriptors_[I], static_cast<typename std::tuple_element_t<I, ArgsTuple>*>(0))...);
	}

	template<typename T>
	static std::enable_if_t<!std::is_pointer_v<T>, T>
	collect_argument(const parameter_sources& sources, const parameters::descriptor& descriptor, const T* const tag)
	{
		return std::visit(
		    [&](auto&& param) -> T {
			    using P = std::decay_t<decltype(param)>;
			    if constexpr (std::is_same_v<P, p::path> && ConvertibleFromStringView<T>)
			    {
				    return collect_path_argument(sources, param, tag);
			    }
			    else if constexpr (std::is_same_v<P, p::query> && ConvertibleFromStringView<T>)
			    {
				    return collect_query_argument(sources, param, tag);
			    }
			    else if constexpr (std::is_same_v<P, p::header> && ConvertibleFromStringView<T>)
			    {
				    return collect_header_argument(sources, param, tag);
			    }
			    else if constexpr (std::is_same_v<P, p::auth> && std::is_base_of_v<auth_data_base, T>)
			    {
				    return collect_auth_argument(sources, param, tag);
			    }
			    else if constexpr (std::is_same_v<P, p::json> && ConvertibleFromBoostJson<T>)
			    {
				    try
				    {
					    return boost::json::value_to<T>(boost::json::parse(sources.req.body()));
				    }
				    catch (const std::exception& e)
				    {
					    ZOO_THROW_EXCEPTION(argument_error{ fmt::format(
					                            "Could not convert request body to type {}: {}", demangled_type_name<T>(), e.what()) }
					                        << boost::errinfo_nested_exception{ boost::current_exception() });
				    }
			    }
			    else if constexpr (std::is_same_v<P, p::request> && std::is_same_v<T, request>)
			    {
				    return std::cref(sources.req);
			    }
			    else if constexpr (std::is_same_v<P, p::url> && std::is_same_v<T, url_view>)
			    {
				    return std::cref(sources.url);
			    }
			    return T{};
		    },
		    descriptor);
	}

	template<typename T>
	static T collect_path_argument(const parameter_sources& sources, const p::path& param, const T* const tag)
	{
		const auto it = sources.param.find(param.name);
		if (it == sources.param.end())
		{
			// Should not happen if the path spec matches the descriptor
			ZOO_THROW_EXCEPTION(argument_error{ fmt::format("Parameter '{}' not found in path", param.name) });
		}
		return collect_string_argument(param.name, it->second, tag);
	}

	template<typename T>
	static T collect_query_argument(const parameter_sources& sources, const p::query& param, const T* const tag)
	{
		const auto it = sources.url.params().find(param.name);
		if (it == sources.url.params().end())
		{
			if constexpr (is_optional_v<T>)
			{
				return T{};
			}
			ZOO_THROW_EXCEPTION(argument_error{ fmt::format("Query parameter '{}' is required", param.name) });
		}
		return collect_string_argument(param.name, (*it).value, tag);
	}

	template<typename T>
	static T collect_auth_argument(const parameter_sources& sources, const p::auth& param, const T* const tag)
	{
		const auto it = sources.auth.find(param.name);
		if (it == sources.auth.end())
		{
			ZOO_THROW_EXCEPTION(argument_error{ fmt::format("Security scheme '{}' has not been verified", param.name) });
		}
		const auto& ptr = it->second;
		if (!ptr)
		{
			return T{};
		}
		return std::cref(dynamic_cast<const T&>(*ptr));
	}

	template<typename T>
	static T collect_header_argument(const parameter_sources& sources, const p::header& param, const T* const tag)
	{
		const auto it = sources.req.find(param.name);
		if (it == sources.req.end())
		{
			if constexpr (is_optional_v<T>)
			{
				return T{};
			}
			ZOO_THROW_EXCEPTION(argument_error{ fmt::format("Header '{}' is required", param.name) });
		}
		return collect_string_argument(param.name, sources.req[param.name], tag);
	}

	template<typename T>
	static T collect_string_argument(std::string_view name, std::string_view value, const T* const tag)
	{
		try
		{
			return conversions::from_string_view(value, tag);
		}
		catch (const std::exception& e)
		{
			ZOO_THROW_EXCEPTION(argument_error{ fmt::format("'{}': could not convert {} to type {}: {}",
			                                                name,
			                                                fmt::streamed(std::quoted(value)),
			                                                demangled_type_name<T>(),
			                                                e.what()) }
			                    << boost::errinfo_nested_exception{ boost::current_exception() });
		}
	}

private:
	Callback                              callback_;
	std::array<parameters::descriptor, N> descriptors_;
};

} // namespace spider
} // namespace zoo
