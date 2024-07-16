//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>
#include <zoo/squid/postgresql/error.h>
#include <zoo/squid/postgresql/detail/pqapimock.h>

namespace zoo {
namespace squid {
namespace postgresql {

TEST(ErrorTests, TestConstructionWithMessage)
{
	auto e = error{ "the message" };

	EXPECT_STREQ(e.what(), "the message");
	EXPECT_FALSE(e.sql_state().has_value());
}

TEST(ErrorTests, TestConstructionWithApiMessageAndConnection)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, errorMessage(testing::Eq(pq_api_mock::test_connection))).WillOnce(testing::Return("the detail for conn"));

	auto e = error{ &api, "the message", *pq_api_mock::test_connection };

	EXPECT_STREQ(e.what(), "the message\nthe detail for conn");
	EXPECT_FALSE(e.sql_state().has_value());
}

TEST(ErrorTests, TestApiErrorMessageReturnsNullptr)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, errorMessage(testing::Eq(pq_api_mock::test_connection))).WillOnce(testing::ReturnNull());

	auto e = error{ &api, "the message", *pq_api_mock::test_connection };

	EXPECT_STREQ(e.what(), "the message");
	EXPECT_FALSE(e.sql_state().has_value());
}

TEST(ErrorTests, TestConstructionWithApiMessageAndConnectionAndResult)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, resultStatus(testing::Eq(pq_api_mock::test_result))).WillOnce(testing::Return(PGRES_FATAL_ERROR));
	EXPECT_CALL(api, resStatus(PGRES_FATAL_ERROR)).WillOnce(testing::Return("the status name"));
	EXPECT_CALL(api, resultErrorMessage(testing::Eq(pq_api_mock::test_result))).WillOnce(testing::Return("the detail for result"));
	EXPECT_CALL(api, errorMessage(testing::Eq(pq_api_mock::test_connection))).Times(0);
	EXPECT_CALL(api, resultErrorField(testing::Eq(pq_api_mock::test_result), PG_DIAG_SQLSTATE)).WillOnce(testing::Return("12345"));

	auto e = error{ &api, "the message", *pq_api_mock::test_connection, *pq_api_mock::test_result };

	EXPECT_STREQ(e.what(), "the message (the status name)\nthe detail for result");
	EXPECT_TRUE(e.sql_state().has_value());
	EXPECT_EQ(e.sql_state().value(), "12345");
}

TEST(ErrorTests, TestConstructionWithApiMessageAndConnectionAndResultWithoutStatus)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, resultStatus(testing::Eq(pq_api_mock::test_result))).WillOnce(testing::Return(PGRES_FATAL_ERROR));
	EXPECT_CALL(api, resStatus(PGRES_FATAL_ERROR)).WillOnce(testing::ReturnNull());
	EXPECT_CALL(api, resultErrorMessage(testing::Eq(pq_api_mock::test_result))).WillOnce(testing::Return("the detail for result"));
	EXPECT_CALL(api, errorMessage(testing::Eq(pq_api_mock::test_connection))).Times(0);
	EXPECT_CALL(api, resultErrorField(testing::Eq(pq_api_mock::test_result), PG_DIAG_SQLSTATE)).WillOnce(testing::Return("12345"));

	auto e = error{ &api, "the message", *pq_api_mock::test_connection, *pq_api_mock::test_result };

	EXPECT_STREQ(e.what(), "the message\nthe detail for result");
	EXPECT_TRUE(e.sql_state().has_value());
	EXPECT_EQ(e.sql_state().value(), "12345");
}

TEST(ErrorTests, TestConstructionWithApiMessageAndConnectionAndResultWithoutResultMessage)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, resultStatus(testing::Eq(pq_api_mock::test_result))).WillOnce(testing::Return(PGRES_FATAL_ERROR));
	EXPECT_CALL(api, resStatus(PGRES_FATAL_ERROR)).WillOnce(testing::Return("the status name"));
	EXPECT_CALL(api, resultErrorMessage(testing::Eq(pq_api_mock::test_result))).WillOnce(testing::ReturnNull());
	EXPECT_CALL(api, errorMessage(testing::Eq(pq_api_mock::test_connection))).WillOnce(testing::Return("the detail for conn"));
	EXPECT_CALL(api, resultErrorField(testing::Eq(pq_api_mock::test_result), PG_DIAG_SQLSTATE)).WillOnce(testing::Return("12345"));

	auto e = error{ &api, "the message", *pq_api_mock::test_connection, *pq_api_mock::test_result };

	EXPECT_STREQ(e.what(), "the message (the status name)\nthe detail for conn");
	EXPECT_TRUE(e.sql_state().has_value());
	EXPECT_EQ(e.sql_state().value(), "12345");
}

TEST(ErrorTests, TestConstructionWithApiMessageAndConnectionAndResultWithoutResultNorConnMessage)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, resultStatus(testing::Eq(pq_api_mock::test_result))).WillOnce(testing::Return(PGRES_FATAL_ERROR));
	EXPECT_CALL(api, resStatus(PGRES_FATAL_ERROR)).WillOnce(testing::Return("the status name"));
	EXPECT_CALL(api, resultErrorMessage(testing::Eq(pq_api_mock::test_result))).WillOnce(testing::ReturnNull());
	EXPECT_CALL(api, errorMessage(testing::Eq(pq_api_mock::test_connection))).WillOnce(testing::ReturnNull());
	EXPECT_CALL(api, resultErrorField(testing::Eq(pq_api_mock::test_result), PG_DIAG_SQLSTATE)).WillOnce(testing::Return("12345"));

	auto e = error{ &api, "the message", *pq_api_mock::test_connection, *pq_api_mock::test_result };

	EXPECT_STREQ(e.what(), "the message (the status name)");
	EXPECT_TRUE(e.sql_state().has_value());
	EXPECT_EQ(e.sql_state().value(), "12345");
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
