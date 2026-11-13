//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/forwarding_request_handler.h"
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
#include <utility>

namespace zoo {
namespace spider {

forwarding_request_handler::forwarding_request_handler(std::shared_ptr<irequest_handler> next)
    : next_{ std::move(next) }
{
}

forwarding_request_handler::~forwarding_request_handler() noexcept
{
}

std::optional<response_wrapper> forwarding_request_handler::handle_request_header(const http::request_header<>& header,
                                                                                  std::optional<auth_map>&      preverified)
{
	return this->next_->handle_request_header(header, preverified);
}

response_wrapper forwarding_request_handler::handle_request(request&& req)
{
	return this->handle_request(std::move(req), std::nullopt);
}

response_wrapper forwarding_request_handler::handle_request(request&& req, std::optional<auth_map>&& preverified)
{
	return this->next_->handle_request(std::move(req), std::move(preverified));
}

response_wrapper forwarding_request_handler::make_error_response(http::status status, std::string_view message)
{
	return this->next_->make_error_response(status, message);
}

} // namespace spider
} // namespace zoo
