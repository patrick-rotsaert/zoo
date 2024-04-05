//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

// Code in this file is inspired by https://github.com/mhekkel/libzeep,
// a very old version of it in fact, I think 3.something.

// Copyright Maarten L. Hekkelman, Radboud University 2008-2012.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)

#ifndef ZOO_SPIDER_DISPATCHER_H

#if !defined(BOOST_PP_IS_ITERATING)

#include "zoo/spider/request_router.h"
#include "zoo/spider/error_response.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/message.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/misc/demangled_type_name.hpp"
#include "zoo/common/misc/throw_exception.h"

#include <boost/version.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>

#include <boost/array.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/accumulate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/json.hpp>
#include <boost/regex.hpp>

#include <fmt/format.h>

#include <map>
#include <memory>
#include <variant>
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

struct json_body final
{
	std::string name;
};

namespace detail {

using parameter_descriptor = std::variant<path_parameter, query_parameter, json_body>;

template<typename T>
void convert_parameter(const beast::string_view& in, T& out)
{
	out = boost::lexical_cast<T>(in);
}

template<typename T>
void convert_parameter(const beast::string_view& in, std::optional<T>& out)
{
	convert_parameter(in, out.emplace());
}

template<typename T>
void convert_parameter(const beast::string_view& in, boost::optional<T>& out)
{
	out.emplace();
	convert_parameter(in, out.value());
}

namespace f = boost::fusion;

template<class... Ts>
struct overloaded : Ts...
{
	using Ts::operator()...;
};

template<typename Iterator>
struct parameter_deserializer final
{
	using result_type = Iterator;
	using svmatch     = boost::match_results<string_view::const_iterator>;

	const request&     req_;
	const url_view&    url_;
	const string_view& path_;
	const svmatch&     match_;

	parameter_deserializer(const request& req, const url_view& url, const string_view& path, const svmatch& match)
	    : req_{ req }
	    , url_{ url }
	    , path_{ path }
	    , match_{ match }
	{
	}

	template<typename T>
	Iterator operator()(Iterator i, T& t) const
	{
		std::visit(
		    [&, this](auto&& p) {
			    using P = std::decay_t<decltype(p)>;
			    if constexpr (std::is_same_v<P, path_parameter> && !ConvertibleFromBoostJson<T>)
			    {
				    const auto& m = this->match_[p.name];
				    if (m.matched)
				    {
					    const auto sv = string_view{ m.first, m.second };
					    try
					    {
						    convert_parameter(sv, t);
					    }
					    catch (const std::exception& e)
					    {
						    ZOO_THROW_EXCEPTION(std::invalid_argument{ fmt::format("'{}': could not convert {} to {}: {}",
						                                                           p.name,
						                                                           fmt::streamed(std::quoted(std::string{ sv })),
						                                                           demangled_type_name(t),
						                                                           e.what()) });
					    }
				    }
				    else
				    {
					    t = T{};
				    }
			    }
			    else if constexpr (std::is_same_v<P, query_parameter> && !ConvertibleFromBoostJson<T>)
			    {
				    const auto pit = this->url_.params().find(p.name);
				    if (pit != this->url_.params().end())
				    {
					    const auto& s = (*pit).value;
					    try
					    {
						    convert_parameter(s, t);
					    }
					    catch (const std::exception& e)
					    {
						    ZOO_THROW_EXCEPTION(std::invalid_argument{ fmt::format("'{}': could not convert {} to {}: {}",
						                                                           p.name,
						                                                           fmt::streamed(std::quoted(s)),
						                                                           demangled_type_name(t),
						                                                           e.what()) });
					    }
				    }
				    else
				    {
					    t = T{};
				    }
			    }
			    else if constexpr (std::is_same_v<P, json_body> && ConvertibleFromBoostJson<T>)
			    {
				    try
				    {
					    t = boost::json::value_to<T>(boost::json::parse(this->req_.body()));
				    }
				    catch (const std::exception& e)
				    {
					    ZOO_THROW_EXCEPTION(std::invalid_argument{
					        fmt::format("'{}': could not convert request body to {}: {}", p.name, demangled_type_name(t), e.what()) });
				    }
			    }
		    },
		    *i);

		return ++i;
	}
};

template<typename Method>
struct handler_traits;

// first specialization, no input arguments specified
template<class Class>
struct handler_traits<message_generator (Class::*)(request&&)>
{
	using argument_type = typename f::vector<>;

	typedef message_generator (Class::*Method)(request&&);

	static message_generator invoke(Class* object, Method method, request&& req, argument_type)
	{
		return (object->*method)(std::move(req));
	}
};

// all the other specializations are specified at the bottom of this file
#define BOOST_PP_FILENAME_1 <zoo/spider/dispatcher.hpp>
#define BOOST_PP_ITERATION_LIMITS (1, 9) // TODO: make the upper limit configurable
#include BOOST_PP_ITERATE()

struct handler_base
{
	using url_view    = boost::urls::url_view;
	using string_view = beast::string_view;
	using svmatch     = boost::match_results<string_view::const_iterator>;

	handler_base()
	{
	}

	virtual ~handler_base()
	{
	}

	virtual message_generator call(request&& req, url_view&& url, string_view path, const svmatch& match) = 0;
};

template<class Class, typename Method>
struct handler final : public handler_base
{
	using argument_type = typename handler_traits<Method>::argument_type;

	enum
	{
		descriptor_count = argument_type::size::value
	};

	using descriptors_type = parameter_descriptor[descriptor_count];

	handler(Class* object, Method method, const descriptors_type& descriptors)
	    : handler_base{}
	    , object_{ object }
	    , method_{ method }
	{
		std::copy(descriptors, descriptors + descriptor_count, this->descriptors_.begin());
	}

	virtual message_generator call(request&& req, url_view&& url, string_view path, const svmatch& match)
	{
		// start by collecting all the parameters
		argument_type args;
		try
		{
			boost::fusion::accumulate(args, descriptors_.begin(), parameter_deserializer<parameter_descriptor*>(req, url, path, match));
		}
		catch (const std::exception& e)
		{
			ZOO_LOG(err, "Could not collect parameters: {}", e.what());
			return bad_request::create(req);
		}

		// now call the actual server code
		return handler_traits<Method>::invoke(object_, method_, std::move(req), args);
	}

	Class*                                               object_;
	Method                                               method_;
	boost::array<parameter_descriptor, descriptor_count> descriptors_;
};

} // namespace detail

struct dispatcher final
{
	using svmatch = boost::match_results<string_view::const_iterator>;

	template<class Class, typename Method>
	static void register_action(const std::shared_ptr<request_router>&                           router,
	                            std::set<verb>&&                                                 methods,
	                            boost::regex&&                                                   pattern,
	                            Class*                                                           object,
	                            Method                                                           method,
	                            const typename detail::handler<Class, Method>::descriptors_type& arg)
	{
		// Note: the closure passed to add_route must be copyable.
		// This is why the handler is created as a shared_ptr, rather than a unique_ptr.
		// I would have preferred to use a unique_ptr, but then the handler would need to be
		// move-captured, thus making the lambda non-copyable and std::function would fail to create.
		auto handler = std::make_shared<detail::handler<Class, Method>>(object, method, arg);
		router->add_route(std::move(methods),
		                  std::move(pattern),
		                  [handler = handler](request&& req, url_view&& url, string_view path, const svmatch& match) {
			                  return handler->call(std::move(req), std::move(url), path, match);
		                  });
	}
};

} // namespace spider
} // namespace zoo

#define ZOO_SPIDER_DISPATCHER_H

#else // BOOST_PP_IS_ITERATING
//
//	Specializations for the handler_traits for a range of parameters
//
#define N BOOST_PP_ITERATION()

template<class Class, BOOST_PP_ENUM_PARAMS(N, typename T)>
struct handler_traits<message_generator (Class::*)(request&&, BOOST_PP_ENUM_PARAMS(N, T))>
{
	typedef message_generator (Class::*Method)(request&&, BOOST_PP_ENUM_PARAMS(N, T));

#define M(z, j, data) typedef typename boost::remove_const<typename boost::remove_reference<T##j>::type>::type t_##j;
	BOOST_PP_REPEAT(N, M, ~)
#undef M

	using argument_type = typename f::vector<BOOST_PP_ENUM_PARAMS(N, t_)>;

#define M(z, j, data) f::at_c<j>(arguments)
	static message_generator invoke(Class* object, Method method, request&& req, argument_type arguments)
	{
		return (object->*method)(std::move(req), BOOST_PP_ENUM(N, M, ~));
	}
#undef M
};

#undef N

#endif
#endif
