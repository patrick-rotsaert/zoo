//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/openapi.h"

#include <boost/algorithm/string/case_conv.hpp>

namespace zoo {
namespace spider {

openapi::openapi()
    : spec_{}
{
	spec_["openapi"] = "3.0.0";
	spec_["info"]    = { { "title", "Generated API" }, { "version", "1.0.0" } }; //@@
}

const boost::json::object& openapi::spec() const
{
	return spec_;
}

std::string openapi::method_name(verb method)
{
	auto        sv = to_string(method);
	std::string name{ sv.begin(), sv.end() };
	boost::algorithm::to_lower(name);
	return name;
}

} // namespace spider
} // namespace zoo
