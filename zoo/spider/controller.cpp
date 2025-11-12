//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/controller.hpp"

namespace zoo {
namespace spider {

controller::exception_handler_base::~exception_handler_base() = default;

controller::argument_error::argument_error(const std::string& m)
    : exception_base{ m }
{
}

controller::~controller() = default;

controller::controller(std::shared_ptr<request_router> router)
    : router_{ std::move(router) }
    , exception_handler_{}
{
}

const std::shared_ptr<request_router>& controller::router() const
{
	return this->router_;
}

void controller::exception_handler(const std::shared_ptr<exception_handler_base>& value)
{
	this->exception_handler_ = value;
}

} // namespace spider
} // namespace zoo
