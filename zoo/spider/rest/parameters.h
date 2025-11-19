//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/rest/pathspec.h"
#include "zoo/spider/message.h"

#include <boost/url/url_view.hpp>

#include <string_view>
#include <variant>

namespace zoo {
namespace spider {

struct parameters final
{
	struct named_parameter
	{
		std::string_view name;
		std::string_view description;
	};

	struct path_parameter final : named_parameter
	{
	};

	struct query_parameter final : named_parameter
	{
	};

	struct header_parameter final : named_parameter
	{
	};

	struct json_body final
	{
	};

	struct request_parameter final
	{
	};

	struct url_parameter final
	{
	};

	struct p final
	{
		using path    = path_parameter;
		using query   = query_parameter;
		using header  = header_parameter;
		using json    = json_body;
		using request = request_parameter;
		using url     = url_parameter;
	};

	using descriptor = std::variant<p::path, p::query, p::header, p::json, p::request, p::url>;
};

struct parameter_sources final
{
	const request&              req;
	const url_view&             url;
	const path_spec::param_map& param;
};

} // namespace spider
} // namespace zoo
