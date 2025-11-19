//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/json_util.h"

#include <boost/json.hpp>

#include <sstream>

namespace zoo {
namespace spider {
namespace {

constexpr auto INDENT = 2;

// https://www.boost.org/doc/libs/1_83_0/libs/json/doc/html/json/examples.html
void pretty_print_impl(std::ostream& os, boost::json::value const& jv, std::string* indent = nullptr)
{
	namespace json = boost::json;

	std::string indent_;
	if (!indent)
	{
		indent = &indent_;
	}
	switch (jv.kind())
	{
	case json::kind::object:
	{
		os << "{\n";
		indent->append(INDENT, ' ');
		auto const& obj = jv.get_object();
		if (!obj.empty())
		{
			auto it = obj.begin();
			for (;;)
			{
				os << *indent << json::serialize(it->key()) << ": ";
				pretty_print_impl(os, it->value(), indent);
				if (++it == obj.end())
				{
					break;
				}
				os << ",\n";
			}
		}
		os << "\n";
		indent->resize(indent->size() - INDENT);
		os << *indent << "}";
		break;
	}

	case json::kind::array:
	{
		os << "[\n";
		indent->append(INDENT, ' ');
		auto const& arr = jv.get_array();
		if (!arr.empty())
		{
			auto it = arr.begin();
			for (;;)
			{
				os << *indent;
				pretty_print_impl(os, *it, indent);
				if (++it == arr.end())
				{
					break;
				}
				os << ",\n";
			}
		}
		os << "\n";
		indent->resize(indent->size() - INDENT);
		os << *indent << "]";
		break;
	}

	case json::kind::string:
	{
		os << json::serialize(jv.get_string());
		break;
	}

	case json::kind::uint64:
	case json::kind::int64:
	case json::kind::double_:
		os << jv;
		break;

	case json::kind::bool_:
		if (jv.get_bool())
		{
			os << "true";
		}
		else
		{
			os << "false";
		}
		break;

	case json::kind::null:
		os << "null";
		break;
	}

	if (indent->empty())
	{
		os << "\n";
	}
}

} // namespace

std::string json_util::pretty_print(const boost::json::value& in)
{
	std::ostringstream os{};
	pretty_print_impl(os, in);
	return os.str();
}

} // namespace spider
} // namespace zoo
