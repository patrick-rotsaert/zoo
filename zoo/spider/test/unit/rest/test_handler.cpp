//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/rest/handler.hpp"
#include "zoo/spider/rest/parameters.h"
#include "zoo/spider/rest/auth.h"
#include "zoo/spider/messages/message.h"

#include <boost/beast/http/verb.hpp>
#include <boost/url/parse.hpp>

#include <string>

namespace zoo {
namespace spider {
namespace {

struct probe_controller
{
	virtual ~probe_controller() = default;

	std::string plain()
	{
		return "plain";
	}
	std::string konst() const
	{
		return "const";
	}
	std::string noex() noexcept
	{
		return "noexcept";
	}
	std::string const_noex() const noexcept
	{
		return "const-noexcept";
	}
};

// Builds a handler for a zero-argument controller method and invokes it.
template<typename Method>
std::string call_handler(probe_controller& ctrl, Method method)
{
	handler<Method>      h{ &ctrl, method };
	request              req{ http::verb::get, "/", 11 };
	const auto           url = boost::urls::parse_origin_form("/").value();
	path_spec::param_map param{};
	auth_map             auth{};
	return h.call(parameter_sources{ req, url, param, auth });
}

} // namespace

// A controller may expose an operation as a const and/or noexcept member function; the handler must
// accept all cv-/noexcept-qualified member-function pointers, not just plain ones.
TEST(RestHandler, AcceptsConstAndNoexceptMemberFunctions)
{
	probe_controller ctrl;

	EXPECT_EQ(call_handler(ctrl, &probe_controller::plain), "plain");
	EXPECT_EQ(call_handler(ctrl, &probe_controller::konst), "const");
	EXPECT_EQ(call_handler(ctrl, &probe_controller::noex), "noexcept");
	EXPECT_EQ(call_handler(ctrl, &probe_controller::const_noex), "const-noexcept");
}

} // namespace spider
} // namespace zoo
