//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/json_response.h"

#include <boost/beast/version.hpp>

namespace zoo {
namespace spider {

response spider::json_response::create_impl(const request& req, http::status status, std::string&& json)
{
	auto res = response{ status, req.version() };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, "application/json");
	res.keep_alive(req.keep_alive());
	res.body() = std::move(json);
	res.prepare_payload();
	return res;
}

response json_response::create_impl(http::status status, std::string&& json)
{
	auto res = response{};
	res.result(status);
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, "application/json");
	res.body() = std::move(json);
	res.prepare_payload();
	return res;
}

} // namespace spider
} // namespace zoo
