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
#include "zoo/spider/messages/response_wrapper.hpp"
#include "zoo/spider/rest/auth.h"

#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>

#include <optional>

namespace zoo {
namespace spider {

class ZOO_SPIDER_API irequest_handler
{
public:
	irequest_handler();
	virtual ~irequest_handler() noexcept;

	irequest_handler(const irequest_handler&)            = delete;
	irequest_handler& operator=(const irequest_handler&) = delete;
	irequest_handler(irequest_handler&&)                 = default;
	irequest_handler& operator=(irequest_handler&&)      = default;

	// Called as soon as the request header is in, before the body is read. A response returned here is sent instead of reading
	// the body, and the connection is closed. Authentication done here is put in `preverified` and passed on to handle_request().
	// A handler that wraps another one (a logger, a filter) must forward all three virtuals, or the wrapped handler's header-phase
	// work is skipped.
	virtual std::optional<response_wrapper> handle_request_header(const http::request_header<>& header,
	                                                              std::optional<auth_map>&      preverified);

	virtual response_wrapper handle_request(request&& req) = 0;

	// Called with the complete request. `preverified` holds what handle_request_header() authenticated, if anything.
	virtual response_wrapper handle_request(request&& req, std::optional<auth_map>&& preverified);
};

} // namespace spider
} // namespace zoo
