//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "data.h"

#include "zoo/spider/aliases.h"
#include "zoo/spider/rest/status_result.hpp"
#include "zoo/spider/content_container.hpp"

#include <boost/optional/optional.hpp>

#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace demo {

class Operations
{
public:
	std::vector<Customer> listCustomers(std::string api_key);

	Customer getCustomer(std::uint64_t                       id,
	                     const std::optional<std::string>&   serial,
	                     const boost::optional<std::string>& api_key,
	                     std::optional<Status>);

	auto postCustomer(const Customer& c)
	{
		return make_status_result<status::created>(createCustomer(c));
	}

	void noop();

	status_result<status::not_implemented, Error> fail(std::optional<bool> useException);

	image_container image();

	std::variant<status_result<status::ok, TestString>,
	             status_result<status::ok, image_container>, // not used, for OAS demo only
	             status_result<status::not_found, std::string>,
	             status_result<status::not_found, Error> // not used, for OAS demo only
	             >
	test(bool found);

private:
	Customer createCustomer(const Customer& c);
};

} // namespace demo
