//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/asyncerror.h"

namespace zoo {
namespace squid {
namespace postgresql {

std::string async_error::format() const
{
	if (message)
	{
		return message.value();
	}
	if (ec)
	{
		return ec->message();
	}
	return "Unspecified error";
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
