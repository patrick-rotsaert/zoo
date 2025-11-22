//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/spider/config.h"
#include "zoo/spider/aliases.h"

#include <boost/exception/all.hpp>
#include <boost/exception/exception.hpp>

#include <string>

namespace zoo {
namespace spider {

using ex_mesg   = boost::error_info<struct ex_mesg_, std::string>;
using ex_code   = boost::error_info<struct ex_code_, int>;
using ex_status = boost::error_info<struct ex_status_, http::status>;

struct ZOO_SPIDER_API exception_base : virtual boost::exception, virtual std::exception
{
	using mesg   = ex_mesg;
	using code   = ex_code;
	using status = ex_status;

	~exception_base() noexcept override;

	exception_base() noexcept;
	explicit exception_base(const std::string& mesg) noexcept;

	const char* what() const noexcept override;
};

template<http::status Status>
struct ZOO_SPIDER_API exception : virtual exception_base
{
	exception() noexcept
	    : exception_base{}
	{
		*this << ex_status{ Status };
	}

	explicit exception(const std::string& mesg) noexcept
	    : exception_base{ mesg }
	{
		*this << ex_status{ Status };
	}
};

using bad_request_exception           = exception<status::bad_request>;
using not_found_exception             = exception<status::not_found>;
using internal_server_error_exception = exception<status::internal_server_error>;
using not_implemented_exception       = exception<status::not_implemented>;

} // namespace spider
} // namespace zoo
