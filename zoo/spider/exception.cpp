//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/exception.h"

#include <boost/exception/get_error_info.hpp>

namespace zoo {
namespace spider {

exception_base::exception_base() noexcept
    : boost::exception{}
    , std::exception{}
{
}

exception_base::exception_base(const std::string& mesg) noexcept
    : boost::exception{}
    , std::exception{}
{
	*this << ex_mesg{ mesg };
}

exception_base::~exception_base() noexcept
{
}

const char* exception_base::what() const noexcept
{
	if (const auto mesg = boost::get_error_info<ex_mesg>(*this))
	{
		return mesg->c_str();
	}
	else
	{
		return std::exception::what();
	}
}

} // namespace spider
} // namespace zoo
