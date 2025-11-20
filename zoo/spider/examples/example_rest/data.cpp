//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "data.h"

#include "zoo/spider/exception.h"
#include "zoo/common/conversion/conversion.h"

#include <boost/json.hpp>
#include <boost/exception/all.hpp>

namespace demo {

Customer Customer::example()
{
	return Customer{ 1234, "The customer name", { Status::sold } };
}

Error Error::create(const std::exception& e)
{
	auto err = Error{};

	err.status_ = status::internal_server_error;
	if (const auto x = boost::get_error_info<ex_status>(e))
	{
		err.status_ = *x;
	}

	err.message = e.what();

	if (const auto x = boost::get_error_info<ex_code>(e))
	{
		err.error_code = *x;
	}

	auto loc = Error::SourceLocation{};
	if (const auto x = boost::get_error_info<boost::throw_file>(e))
	{
		loc.file = *x;
	}
	if (const auto x = boost::get_error_info<boost::throw_function>(e))
	{
		loc.function = *x;
	}
	if (const auto x = boost::get_error_info<boost::throw_line>(e))
	{
		loc.line = *x;
	}
	if (loc.file || loc.function || loc.line)
	{
		err.location = std::move(loc);
	}

	return err;
}

Error Error::create(int error_code, std::string message)
{
	auto err       = Error{};
	err.message    = std::move(message);
	err.error_code = error_code;
	return err;
}

Error Error::example()
{
	auto err       = Error{};
	err.message    = "The error message";
	err.error_code = 42;
	err.location.emplace("source.cpp", "some_function", 999);
	return err;
}

zoo::spider::http::status Error::status() const noexcept
{
	return status_;
}

template<>
TestString TestString::example()
{
	return TestString{ true, "The test string" };
}

template<>
TestUuid TestUuid::example()
{
	return TestUuid{ true, zoo::conversion::string_to_uuid("969fbc80-ab71-45b2-b34c-d211890823d0") };
}

template<>
std::string_view TestString::type_name()
{
	return "TestString";
}

template<>
std::string_view TestUuid::type_name()
{
	return "TestUuid";
}

} // namespace demo
