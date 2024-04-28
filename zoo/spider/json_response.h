//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/message.h"

#include <boost/json.hpp>

#include <concepts>

namespace zoo {
namespace spider {

// FIXME: this should use a type trait like is_tag_invoked or similar,
// but I did not yet find a way to do it.
// Settle for is_described_class now.
template<typename T>
concept ConvertibleToBoostJson = boost::json::is_described_class<T>::value;

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

	template<typename T>
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
	static response create(http::status status, T&& data)
	{
		return create_impl(status, boost::json::serialize(boost::json::value_from(std::move(data))));
	}
};

} // namespace spider
} // namespace zoo
