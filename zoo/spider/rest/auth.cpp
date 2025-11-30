//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/rest/auth.h"
#include "zoo/common/misc/quoted_c.h"

#include <boost/algorithm/string/join.hpp>
#include <fmt/format.h>

namespace zoo {
namespace spider {
namespace {

std::string encode_params(const std::vector<std::pair<std::string_view, std::string>>& params)
{
	std::vector<std::string> parts;
	for (const auto& pair : params)
	{
		parts.push_back(fmt::format("{}={}", pair.first, quoted_c(pair.second)));
	}
	return boost::algorithm::join(parts, ", ");
}

} // namespace

// ============== //
// auth_data_base //
// ============== //

// auth_data_base::auth_data_base()  = default;
auth_data_base::~auth_data_base() = default;

// ================ //
// www_authenticate //
// ================ //

std::string www_authenticate::encode(const challenge& c)
{
	std::vector<std::string> parts;
	parts.push_back(std::string{ c.auth_scheme });
	auto params = encode_params(c.auth_params);
	if (!params.empty())
	{
		parts.push_back(std::move(params));
	}
	return boost::algorithm::join(parts, " ");
}

} // namespace spider
} // namespace zoo
