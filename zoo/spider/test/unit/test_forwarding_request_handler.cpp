//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/forwarding_request_handler.h"
#include "zoo/spider/aliases.h"
#include "zoo/spider/irequest_handler.h"
#include "zoo/spider/messages/message.h"
#include "zoo/spider/messages/response_wrapper.hpp"
#include "zoo/spider/rest/auth.h"

#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace zoo {
namespace spider {
namespace {

// Counts the calls it gets and answers each with a response that says which virtual made it.
class recording_handler final : public irequest_handler
{
public:
	int  header_count{ 0 };
	int  one_arg_count{ 0 };
	int  two_arg_count{ 0 };
	bool preverified_seen{ false };

	std::optional<response_wrapper> handle_request_header(const http::request_header<>&, std::optional<auth_map>&) override
	{
		++this->header_count;
		return response{ http::status::unauthorized, 11 };
	}

	response_wrapper handle_request(request&&) override
	{
		++this->one_arg_count;
		return make_response(http::status::ok, "one-arg");
	}

	response_wrapper handle_request(request&&, std::optional<auth_map>&& preverified) override
	{
		++this->two_arg_count;
		this->preverified_seen = preverified.has_value();
		return make_response(http::status::ok, "two-arg");
	}

	response_wrapper make_error_response(http::status status, std::string_view message) override
	{
		return make_response(status, std::string{ message });
	}

private:
	static response make_response(http::status status, std::string body)
	{
		auto res   = response{ status, 11 };
		res.body() = std::move(body);
		res.prepare_payload();
		return res;
	}
};

request make_request()
{
	return request{ http::verb::get, "/", 11 };
}

} // namespace

TEST(ForwardingRequestHandler, ForwardsTheHeaderPhase)
{
	auto recorder = std::make_shared<recording_handler>();
	auto base     = forwarding_request_handler{ recorder };

	const auto req         = make_request();
	auto       preverified = std::optional<auth_map>{};
	const auto wrapper     = base.handle_request_header(req.base(), preverified);

	ASSERT_TRUE(wrapper.has_value());
	EXPECT_EQ(std::get<response>(wrapper->value()).result(), http::status::unauthorized);
	EXPECT_EQ(recorder->header_count, 1);
}

TEST(ForwardingRequestHandler, ForwardsARequestWithItsPreverification)
{
	auto recorder = std::make_shared<recording_handler>();
	auto base     = forwarding_request_handler{ recorder };

	const auto wrapper = base.handle_request(make_request(), std::optional<auth_map>{ auth_map{} });

	EXPECT_EQ(std::get<response>(wrapper.value()).body(), "two-arg");
	EXPECT_TRUE(recorder->preverified_seen);
}

TEST(ForwardingRequestHandler, SendsAPlainRequestThroughTheTwoArgumentOverload)
{
	auto recorder = std::make_shared<recording_handler>();
	auto base     = forwarding_request_handler{ recorder };

	const auto wrapper = base.handle_request(make_request());

	EXPECT_EQ(std::get<response>(wrapper.value()).body(), "two-arg");
	EXPECT_EQ(recorder->two_arg_count, 1);
	EXPECT_EQ(recorder->one_arg_count, 0);
	EXPECT_FALSE(recorder->preverified_seen);
}

TEST(ForwardingRequestHandler, ForwardsMakeErrorResponse)
{
	auto recorder = std::make_shared<recording_handler>();
	auto base     = forwarding_request_handler{ recorder };

	const auto wrapper = base.make_error_response(http::status::payload_too_large, "too big");

	const auto& res = std::get<response>(wrapper.value());
	EXPECT_EQ(res.result(), http::status::payload_too_large);
	EXPECT_EQ(res.body(), "too big");
}

} // namespace spider
} // namespace zoo
