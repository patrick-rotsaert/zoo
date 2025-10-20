//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <system_error>
#include <string>
#include <optional>

namespace zoo {
namespace squid {
namespace postgresql {

struct async_error
{
	std::optional<std::error_code>  ec;
	std::optional<std::string>      message;
	std::optional<std::string_view> func;

	std::string format() const;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
