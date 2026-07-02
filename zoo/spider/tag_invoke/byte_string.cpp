//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/tag_invoke/byte_string.h"
#include "zoo/spider/rest/base64.h"

#include <boost/json/value.hpp>

#include <stdexcept>

namespace boost::json {

void ZOO_SPIDER_API tag_invoke(const boost::json::value_from_tag&, boost::json::value& out, const zoo::byte_string& in)
{
	out = zoo::spider::base64::encode(std::string_view{ reinterpret_cast<const char*>(in.data()), in.length() });
}

zoo::byte_string ZOO_SPIDER_API tag_invoke(const boost::json::value_to_tag<zoo::byte_string>&, const boost::json::value& in)
{
	auto binary = zoo::spider::base64::decode_to_string(in.as_string());
	if (!binary)
	{
		// Reject malformed input instead of silently yielding an empty byte_string (which hid bad
		// input from the caller and was inconsistent with the other value_to converters).
		throw std::invalid_argument{ "byte_string: value is not valid base64" };
	}
	return zoo::byte_string{ reinterpret_cast<const zoo::byte_string::value_type*>(binary->data()), binary->length() };
}

} // namespace boost::json
