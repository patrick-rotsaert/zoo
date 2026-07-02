//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/messages/message.h"

#include <boost/json.hpp>

#include <type_traits>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API json_response final
{
	static response create_impl(const request& req, http::status status, std::string&& json);
	static response create_impl(http::status status, std::string&& json);

public:
	template<typename T>
	static response create(const request& req, http::status status, const T& data)
	{
		return create_impl(req, status, boost::json::serialize(boost::json::value_from(data)));
	}

	// Rvalue-only: without the constraint this forwarding-reference overload also binds non-const
	// lvalues (out-ranking the const T& overload) and value_from(std::move(data)) would move from the
	// caller's named object.
	template<typename T>
	    requires(!std::is_lvalue_reference_v<T>)
	static response create(const request& req, http::status status, T&& data)
	{
		return create_impl(req, status, boost::json::serialize(boost::json::value_from(std::move(data))));
	}

	template<typename T>
	static response create(http::status status, const T& data)
	{
		return create_impl(status, boost::json::serialize(boost::json::value_from(data)));
	}

	template<typename T>
	    requires(!std::is_lvalue_reference_v<T>)
	static response create(http::status status, T&& data)
	{
		return create_impl(status, boost::json::serialize(boost::json::value_from(std::move(data))));
	}

	static response create(const request& req, http::status status, const boost::json::object& data);
	static response create(const request& req, http::status status, const boost::json::value& data);
	static response create(http::status status, const boost::json::value& data);
};

} // namespace spider
} // namespace zoo
