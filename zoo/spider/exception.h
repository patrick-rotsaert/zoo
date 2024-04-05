//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/aliases.h"
#include "zoo/common/api.h"

#include <boost/exception/all.hpp>
#include <boost/exception/exception.hpp>

#include <string>

namespace zoo {
namespace spider {

using ex_mesg   = boost::error_info<struct ex_mesg_, std::string>;
using ex_code   = boost::error_info<struct ex_code_, int>;
using ex_status = boost::error_info<struct ex_status_, http::status>;

struct ZOO_EXPORT exception_base : virtual boost::exception, virtual std::exception
{
	using mesg   = ex_mesg;
	using code   = ex_code;
	using status = ex_status;

	~exception_base() noexcept override;

	exception_base() noexcept;
	explicit exception_base(const std::string& mesg) noexcept;

	const char* what() const noexcept override;
};

} // namespace spider
} // namespace zoo
