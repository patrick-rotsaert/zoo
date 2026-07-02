//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/messages/buffer_response.h"

#include <boost/beast/version.hpp>

namespace zoo {
namespace spider {

buffer_response::response
buffer_response::create(const request& req, http::status status, std::string_view content_type, byte_string_view body)
{
	auto res = response{ status, req.version() };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, content_type);
	res.keep_alive(req.keep_alive());
	res.body().data = const_cast<void*>(reinterpret_cast<const void*>(body.data()));
	res.body().size = body.size();
	res.body().more = false;
	res.prepare_payload();
	return res;
}

buffer_response::response buffer_response::create(http::status status, std::string_view content_type, byte_string_view body)
{
	auto res = response{};
	res.result(status);
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, content_type);
	res.body().data = const_cast<void*>(reinterpret_cast<const void*>(body.data()));
	res.body().size = body.size();
	res.body().more = false;
	res.prepare_payload();
	return res;
}

} // namespace spider
} // namespace zoo
