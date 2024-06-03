//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>
#include <zoo/squid/postgresql/backendconnection.h>
#include <zoo/squid/postgresql/detail/pqapimock.h>
#include <zoo/squid/postgresql/statement.h>

namespace zoo {
namespace squid {
namespace postgresql {

namespace {

static constexpr auto g_connection_info = "the connection info";
static constexpr auto g_query           = "select foo from bar";

} // namespace

TEST(BackendConnectionTests, TestOpenAndClose)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, connectdb(testing::StrEq(g_connection_info))).WillOnce(testing::Return(pq_api_mock::test_connection));
	EXPECT_CALL(api, status(pq_api_mock::test_connection)).WillOnce(testing::Return(CONNECTION_OK));
	EXPECT_CALL(api, finish(pq_api_mock::test_connection)).Times(1);

	auto c = backend_connection{ &api, g_connection_info };
	EXPECT_EQ(&c.handle(), pq_api_mock::test_connection);
}

TEST(BackendConnectionTests, TestConnectReturnsNull)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, connectdb(testing::StrEq(g_connection_info))).WillOnce(testing::ReturnNull());
	EXPECT_ANY_THROW((backend_connection{ &api, g_connection_info }));
}

TEST(BackendConnectionTests, TestStatusReturnsNotOk)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, connectdb(testing::StrEq(g_connection_info))).WillOnce(testing::Return(pq_api_mock::test_connection));
	EXPECT_CALL(api, status(pq_api_mock::test_connection)).WillOnce(testing::Return(CONNECTION_BAD));
	EXPECT_CALL(api, finish(pq_api_mock::test_connection)).Times(1);

	EXPECT_ANY_THROW((backend_connection{ &api, g_connection_info }));
}

TEST(BackendConnectionTests, TestExecuteQuery)
{
	auto api = pq_api_mock_nice{};

	auto seq = testing::Sequence{};

	EXPECT_CALL(api, connectdb(testing::StrEq(g_connection_info))).InSequence(seq).WillOnce(testing::Return(pq_api_mock::test_connection));
	EXPECT_CALL(api, status(pq_api_mock::test_connection)).InSequence(seq).WillRepeatedly(testing::Return(CONNECTION_OK));
	EXPECT_CALL(api, exec(pq_api_mock::test_connection, testing::StrEq(g_query)))
	    .InSequence(seq)
	    .WillOnce(testing::Return(pq_api_mock::test_result));
	EXPECT_CALL(api, resultStatus(pq_api_mock::test_result)).InSequence(seq).WillOnce(testing::Return(PGRES_TUPLES_OK));
	EXPECT_CALL(api, clear(pq_api_mock::test_result)).Times(1).InSequence(seq);
	EXPECT_CALL(api, finish(pq_api_mock::test_connection)).Times(1).InSequence(seq);

	backend_connection{ &api, g_connection_info }.execute(g_query);
}

TEST(BackendConnectionTests, TestCreateStatement)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, connectdb(testing::StrEq(g_connection_info))).WillOnce(testing::Return(pq_api_mock::test_connection));
	EXPECT_CALL(api, status(pq_api_mock::test_connection)).WillOnce(testing::Return(CONNECTION_OK));
	EXPECT_CALL(api, finish(pq_api_mock::test_connection)).Times(1);

	auto virtstmt = backend_connection{ &api, g_connection_info }.create_statement(g_query);
	auto stmt     = dynamic_cast<statement*>(virtstmt.get());
	EXPECT_NE(stmt, nullptr);
}

TEST(BackendConnectionTests, TestCreatePreparedStatement)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, connectdb(testing::StrEq(g_connection_info))).WillOnce(testing::Return(pq_api_mock::test_connection));
	EXPECT_CALL(api, status(pq_api_mock::test_connection)).WillOnce(testing::Return(CONNECTION_OK));
	EXPECT_CALL(api, finish(pq_api_mock::test_connection)).Times(1);

	auto virtstmt = backend_connection{ &api, g_connection_info }.create_prepared_statement(g_query);
	auto stmt     = dynamic_cast<statement*>(virtstmt.get());
	EXPECT_NE(stmt, nullptr);
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
