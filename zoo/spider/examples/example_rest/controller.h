//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "data.h"
#include "operations.h"

#include "zoo/spider/rest/controller.hpp"

namespace demo {

class Controller final : public zoo::spider::rest_controller<Error>, public Operations
{
public:
	explicit Controller();

	std::string openApiSpec() const;

private:
	response getOpenApi(const request& req);
};

} // namespace demo
