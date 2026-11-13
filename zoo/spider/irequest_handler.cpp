//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/messages/error_response.h"
#include "zoo/spider/messages/message.h"
#include "zoo/spider/messages/response_wrapper.hpp"
#include "zoo/spider/rest/auth.h"

#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>

#include <optional>
#include <string_view>
#include <utility>

namespace zoo {
namespace spider {

irequest_handler::irequest_handler()
{
}

irequest_handler::~irequest_handler() noexcept
{
}

std::optional<response_wrapper> irequest_handler::handle_request_header(const http::request_header<>&, std::optional<auth_map>&)
{
	return std::nullopt;
}

response_wrapper irequest_handler::handle_request(request&& req, std::optional<auth_map>&&)
{
	return this->handle_request(std::move(req));
}

response_wrapper irequest_handler::make_error_response(http::status status, std::string_view)
{
	return error_response_factory::create(status);
}

} // namespace spider
} // namespace zoo
