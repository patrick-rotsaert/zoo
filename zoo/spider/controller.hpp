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
#include "zoo/spider/request_router.h"
#include "zoo/spider/error_response.h"
#include "zoo/spider/response_wrapper.hpp"
#include "zoo/spider/exception.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/misc/demangled_type_name.hpp"
#include "zoo/common/conversion/conversion.h"

#include <boost/beast/core/string.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/exception/errinfo_nested_exception.hpp>

#include <fmt/format.h>

#include <type_traits>
#include <tuple>
#include <string>
#include <variant>
#include <array>
#include <iomanip>

namespace zoo {
namespace spider {

struct path_parameter final
{
	std::string name;
};

struct query_parameter final
{
	std::string name;
};

struct header_parameter final
{
	std::string name;
};

struct json_body final
{
	std::string name;
};

struct request_parameter final
{
};

struct url_parameter final
{
};

class ZOO_SPIDER_API controller
{
public:
	struct p final
	{
		using path    = path_parameter;
		using query   = query_parameter;
		using header  = header_parameter;
		using json    = json_body;
		using request = request_parameter;
		using url     = url_parameter;
	};

	class ZOO_SPIDER_API exception_handler_base
	{
	public:
		virtual ~exception_handler_base();

		virtual response handle(const std::exception& e, const request& req) = 0;
	};

private:
	using svmatch = boost::match_results<string_view::const_iterator>;

	using parameter_descriptor = std::variant<p::path, p::query, p::header, p::json, p::request, p::url>;

	using time_point  = conversion::time_point;
	using date        = conversion::date;
	using time_of_day = conversion::time_of_day;

	struct parameter_sources final
	{
		const request&     req;
		const url_view&    url;
		const string_view& path;
		const svmatch&     match;
	};

	class ZOO_SPIDER_API argument_error : public exception_base
	{
	public:
		explicit argument_error(const std::string& m);
	};

	template<typename Controller, typename...>
	struct handler
	{
	};

	template<typename Controller, typename Result, typename... Args>
	struct handler<Result (Controller::*)(Args...)>
	{
		using Method     = Result (Controller::*)(Args...);
		using ArgsTuple  = std::tuple<typename std::remove_const_t<typename std::remove_reference_t<Args>>...>;
		using ResultType = typename std::remove_const_t<typename std::remove_reference_t<Result>>;
		using Callback   = std::function<ResultType(Args...)>;

		static constexpr size_t N = sizeof...(Args);

		Callback                            callback_;
		std::array<parameter_descriptor, N> descriptors_;

		template<typename... Descriptors>
		handler(controller* owner, Method method, Descriptors... descriptors)
		    : callback_{}
		    , descriptors_{}
		{
			static_assert(sizeof...(descriptors) == sizeof...(Args),
			              "The number of descriptors must be equal to the number of arguments of the callback function");

			auto controller = dynamic_cast<Controller*>(owner);
			if (controller == nullptr)
			{
				ZOO_THROW_EXCEPTION(std::invalid_argument{ "Callback must be provided by a class derived from controller" });
			}

			this->callback_ = [controller, method](Args... args) { return (controller->*method)(args...); };

			if constexpr (sizeof...(descriptors) > 0)
			{
				auto i = size_t{ 0 };
				for (auto name : { parameter_descriptor{ descriptors }... })
				{
					this->descriptors_[i++] = name;
				}
			}

			validate_descriptors(std::make_index_sequence<N>());
		}

		template<std::size_t... I>
		void validate_descriptors(std::index_sequence<I...>)
		{
			((validate_descriptor<typename std::tuple_element_t<I, ArgsTuple>>(this->descriptors_[I])), ...);
		}

		template<typename T>
		void validate_descriptor(const parameter_descriptor& descriptor)
		{
			auto ok = false;
			if constexpr (std::is_same_v<T, request>)
			{
				ok = std::holds_alternative<p::request>(descriptor);
			}
			else if constexpr (std::is_same_v<T, url_view>)
			{
				ok = std::holds_alternative<p::url>(descriptor);
			}
			else if constexpr (ConvertibleFromBoostJson<T>)
			{
				ok = std::holds_alternative<p::json>(descriptor);
			}
			else
			{
				ok = std::holds_alternative<p::path>(descriptor) || std::holds_alternative<p::query>(descriptor) ||
				     std::holds_alternative<p::header>(descriptor);
			}

			if (!ok)
			{
				const auto descriptor_type_name = std::visit([](auto&& arg) { return demangled_type_name(arg); }, descriptor);
				ZOO_THROW_EXCEPTION(std::logic_error{ fmt::format("Descriptor type {} is incompatible with argument type {} for method {}",
				                                                  descriptor_type_name,
				                                                  demangled_type_name<T>(),
				                                                  demangled_type_name<Method>()) });
			}
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
				// All other exception types will not be catched.
				// Set the http status in the exception
				e << ex_status{ status::bad_request };
				throw;
			}
		}

		template<typename ArgsTuple>
		ResultType invoke(ArgsTuple&& args)
		{
			return std::apply(this->callback_, std::forward<ArgsTuple>(args));
		}

		template<std::size_t... I>
		ArgsTuple collect_arguments(const parameter_sources& sources, std::index_sequence<I...>)
		{
			return std::make_tuple(collect_argument<typename std::tuple_element_t<I, ArgsTuple>>(
			    sources, this->descriptors_[I], static_cast<typename std::tuple_element_t<I, ArgsTuple>*>(0))...);
		}

		template<typename T>
		std::enable_if_t<!std::is_pointer_v<T>, T>
		collect_argument(const parameter_sources& sources, const parameter_descriptor& descriptor, const T* const tag)
		{
			auto [found, name, value] = std::visit(
			    [&](auto&& p)
			        -> std::tuple<bool, std::string_view, std::variant<string_view, std::string_view, std::string, std::nullptr_t>> {
				    using P = std::decay_t<decltype(p)>;
				    if constexpr (std::is_same_v<P, p::path>)
				    {
					    const auto& m = sources.match[p.name];
					    if (m.matched)
					    {
						    return std::make_tuple(true, std::string_view{ p.name }, std::string_view{ m.first, m.second });
					    }
					    else
					    {
						    return std::make_tuple(false, std::string_view{ p.name }, nullptr);
					    }
				    }
				    else if constexpr (std::is_same_v<P, p::query>)
				    {
					    const auto it = sources.url.params().find(p.name);
					    if (it != sources.url.params().end())
					    {
						    return std::make_tuple(true, std::string_view{ p.name }, (*it).value);
					    }
					    else
					    {
						    return std::make_tuple(false, std::string_view{ p.name }, nullptr);
					    }
				    }
				    else if constexpr (std::is_same_v<P, p::header>)
				    {
					    const auto it = sources.req.find(p.name);
					    if (it != sources.req.end())
					    {
						    return std::make_tuple(true, std::string_view{ p.name }, sources.req[p.name]);
					    }
					    else
					    {
						    return std::make_tuple(false, std::string_view{ p.name }, nullptr);
					    }
				    }
				    else if constexpr (std::is_same_v<P, p::json>)
				    {
					    return std::make_tuple(true, std::string_view{ p.name }, nullptr);
				    }
				    return std::make_tuple(false, std::string_view{}, nullptr);
			    },
			    descriptor);

			if constexpr (std::is_same_v<T, request>)
			{
				assert(std::holds_alternative<p::request>(descriptor));
				return std::cref(sources.req);
			}
			else if constexpr (std::is_same_v<T, url_view>)
			{
				assert(std::holds_alternative<p::url>(descriptor));
				return std::cref(sources.url);
			}
			else if constexpr (ConvertibleFromBoostJson<T>)
			{
				assert(std::holds_alternative<p::json>(descriptor));
				try
				{
					return boost::json::value_to<T>(boost::json::parse(sources.req.body()));
				}
				catch (const std::exception& e)
				{
					ZOO_THROW_EXCEPTION(
					    argument_error{
					        fmt::format("'{}': could not convert request body to type {}: {}", name, demangled_type_name<T>(), e.what()) }
					    << boost::errinfo_nested_exception{ boost::current_exception() });
				}
			}
			else if (found)
			{
				auto sv = std::visit(
				    [&](auto&& arg) -> std::string_view {
					    using V = std::decay_t<decltype(arg)>;
					    if constexpr (!std::is_same_v<V, std::nullptr_t>)
					    {
						    return arg;
					    }
					    else
					    {
						    return {}; // should not happen
					    }
				    },
				    value);
				try
				{
					return get_argument(sv, tag);
				}
				catch (const std::exception& e)
				{
					ZOO_THROW_EXCEPTION(argument_error{ fmt::format("'{}': could not convert {} to type {}: {}",
					                                                name,
					                                                fmt::streamed(std::quoted(sv)),
					                                                demangled_type_name<T>(),
					                                                e.what()) }
					                    << boost::errinfo_nested_exception{ boost::current_exception() });
				}
			}
			else
			{
				return T{};
			}
		}

		std::string get_argument(std::string_view in, const std::string* const)
		{
			return std::string{ in };
		}

		bool get_argument(std::string_view in, const bool* const)
		{
			if (beast::iequals(in, "true") || beast::iequals(in, "yes") || beast::iequals(in, "on"))
			{
				return true;
			}
			else if (beast::iequals(in, "false") || beast::iequals(in, "no") || beast::iequals(in, "off"))
			{
				return false;
			}
			else
			{
				return conversion::string_to_number<int>(in) != 0;
			}
		}

		boost::gregorian::date get_argument(std::string_view in, const boost::gregorian::date* const)
		{
			return conversion::string_to_boost_date(in);
		}

		boost::posix_time::time_duration get_argument(std::string_view in, const boost::posix_time::time_duration* const)
		{
			return conversion::string_to_boost_time_duration(in);
		}

		boost::posix_time::ptime get_argument(std::string_view in, const boost::posix_time::ptime* const)
		{
			return conversion::string_to_boost_ptime(in);
		}

		date get_argument(std::string_view in, const date* const)
		{
			return conversion::string_to_date(in);
		}

		time_of_day get_argument(std::string_view in, const time_of_day* const)
		{
			return conversion::string_to_time_of_day(in);
		}

		time_point get_argument(std::string_view in, const time_point* const)
		{
			return conversion::string_to_time_point(in);
		}

		template<typename T>
		std::enable_if_t<std::is_scalar_v<T>, T> get_argument(std::string_view in, const T* const)
		{
			return conversion::string_to_number<T>(in);
		}

		template<typename T>
		std::optional<T> get_argument(std::string_view in, const std::optional<T>* const)
		{
			return std::make_optional<T>(get_argument(in, static_cast<T*>(0)));
		}

		template<typename T>
		boost::optional<T> get_argument(std::string_view in, const boost::optional<T>* const)
		{
			return boost::make_optional(get_argument(in, static_cast<T*>(0)));
		}
	};

	std::shared_ptr<request_router>         router_;
	std::shared_ptr<exception_handler_base> exception_handler_;

public:
	virtual ~controller();

	explicit controller(std::shared_ptr<request_router> router);

	const std::shared_ptr<request_router>& router() const;

	void exception_handler(const std::shared_ptr<exception_handler_base>& value);

protected:
	template<class Callback, typename... ArgDescriptors>
	void register_action(std::set<verb>&& methods, boost::regex&& pattern, Callback callback, ArgDescriptors... descriptors)
	{
		// Note: the closure passed to add_route must be copyable.
		// This is why the handler is created as a shared_ptr, rather than a unique_ptr.
		// I would have preferred to use a unique_ptr, but then the handler would need to be
		// move-captured, thus making the lambda non-copyable and std::function would fail to create.
		auto h = std::make_shared<handler<Callback>>(this, callback, descriptors...);
		this->router_->add_route(
		    std::move(methods),
		    std::move(pattern),
		    [handler = h, this](request&& req, url_view&& url, string_view path, const svmatch& match) -> response_wrapper {
			    try
			    {
				    auto res = handler->call(parameter_sources{ req, url, path, match });
				    res.version(req.version());
				    res.keep_alive(req.keep_alive());
				    return res;
			    }
			    catch (const std::exception& e)
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
		    });
	}
};

} // namespace spider
} // namespace zoo
