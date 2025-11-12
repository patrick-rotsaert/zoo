//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/controller2.hpp"

namespace zoo {
namespace spider {

controller2::exception_handler_base::~exception_handler_base() = default;

controller2::~controller2() = default;

controller2::controller2(std::shared_ptr<request_router2> router)
    : router_{ std::move(router) }
    , exception_handler_{} // , openapi_routes_{}
    , oas_{}
{
}

const std::shared_ptr<request_router2>& controller2::router() const
{
	return this->router_;
}

void controller2::exception_handler(const std::shared_ptr<exception_handler_base>& value)
{
	this->exception_handler_ = value;
}

} // namespace spider
} // namespace zoo
