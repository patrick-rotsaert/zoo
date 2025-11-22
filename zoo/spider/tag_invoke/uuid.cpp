//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/tag_invoke/uuid.h"
#include "zoo/common/conversion/conversion.h"

namespace boost::uuids {

void tag_invoke(const boost::json::value_from_tag&, boost::json::value& out, const uuid& in)
{
	out = zoo::conversion::uuid_to_string(in);
}

uuid tag_invoke(const boost::json::value_to_tag<uuid>&, const boost::json::value& in)
{
	return zoo::conversion::string_to_uuid(in.as_string());
}

} // namespace boost::uuids
