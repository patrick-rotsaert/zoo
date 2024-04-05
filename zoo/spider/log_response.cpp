//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/log_response.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

#include <boost/beast/http.hpp>

namespace zoo {
namespace spider {

namespace {

template<class Body>
inline auto&& log_response_impl(http::response<Body>&& res)
{
	ZOO_LOG(trace, "response:\n{}", fmt::streamed(res));
	return std::move(res);
}

} // namespace

http::response<http::string_body>&& log_response(http::response<http::string_body>&& res)
{
	return log_response_impl(std::move(res));
}

http::response<http::empty_body>&& log_response(http::response<http::empty_body>&& res)
{
	return log_response_impl(std::move(res));
}

} // namespace spider
} // namespace zoo
