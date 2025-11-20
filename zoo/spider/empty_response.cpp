//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/empty_response.h"

#include <boost/beast/version.hpp>

namespace zoo {
namespace spider {

empty_response::empty empty_response::create(const request& req, http::status status)
{
	auto res = empty{ status, req.version() };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.keep_alive(req.keep_alive());
	res.prepare_payload();
	return res;
}

empty_response::empty empty_response::create(http::status status)
{
	auto res = empty{};
	res.result(status);
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.prepare_payload();
	return res;
}

} // namespace spider
} // namespace zoo
