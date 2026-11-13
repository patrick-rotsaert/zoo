//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/messages/message.h"
#include "zoo/spider/messages/response_wrapper.hpp"
#include "zoo/spider/rest/auth.h"

#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>

#include <memory>
#include <optional>
#include <string_view>

namespace zoo {
namespace spider {

// A request handler that wraps another one and forwards every call to it. A wrapper (a logger, a filter) derives from this and
// overrides only what it adds to, calling the base implementation to reach the wrapped handler; a virtual added to
// irequest_handler later is then forwarded without the wrapper changing.
class ZOO_SPIDER_API forwarding_request_handler : public irequest_handler
{
public:
	explicit forwarding_request_handler(std::shared_ptr<irequest_handler> next);
	~forwarding_request_handler() noexcept override;

	std::optional<response_wrapper> handle_request_header(const http::request_header<>& header,
	                                                      std::optional<auth_map>&      preverified) override;

	// Goes through the two-argument overload, so a wrapper that overrides that one sees every request.
	response_wrapper handle_request(request&& req) override;

	response_wrapper handle_request(request&& req, std::optional<auth_map>&& preverified) override;

	response_wrapper make_error_response(http::status status, std::string_view message) override;

private:
	std::shared_ptr<irequest_handler> next_;
};

} // namespace spider
} // namespace zoo
