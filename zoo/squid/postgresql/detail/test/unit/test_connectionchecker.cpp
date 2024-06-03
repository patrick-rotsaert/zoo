//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>
#include <zoo/squid/postgresql/detail/connectionchecker.h>
#include <zoo/squid/postgresql/detail/pqapimock.h>
#include <libpq-fe.h>

namespace zoo {
namespace squid {
namespace postgresql {

TEST(PostgresqlConnectionCheckerTest, TestConnectionOk)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, status(pq_api_mock::test_connection)).WillOnce(testing::Return(CONNECTION_OK));

	EXPECT_EQ(connection_checker::check(&api, pq_api_mock::test_connection), pq_api_mock::test_connection);
}

TEST(PostgresqlConnectionCheckerTest, TestConnectionBadAndOkAfterReset)
{
	auto api = pq_api_mock_nice{};

	{
		auto seq = testing::Sequence{};

		EXPECT_CALL(api, status(pq_api_mock::test_connection)).InSequence(seq).WillOnce(testing::Return(CONNECTION_BAD));
		EXPECT_CALL(api, reset(pq_api_mock::test_connection)).Times(1).InSequence(seq);
		EXPECT_CALL(api, status(pq_api_mock::test_connection)).InSequence(seq).WillOnce(testing::Return(CONNECTION_OK));
	}

	EXPECT_EQ(connection_checker::check(&api, pq_api_mock::test_connection), pq_api_mock::test_connection);
}

TEST(PostgresqlConnectionCheckerTest, TestConnectionBadAndStillBadAfterReset)
{
	auto api = pq_api_mock_nice{};

	{
		auto seq = testing::Sequence{};

		EXPECT_CALL(api, status(pq_api_mock::test_connection)).InSequence(seq).WillOnce(testing::Return(CONNECTION_BAD));
		EXPECT_CALL(api, reset(pq_api_mock::test_connection)).Times(1).InSequence(seq);
		EXPECT_CALL(api, status(pq_api_mock::test_connection)).InSequence(seq).WillOnce(testing::Return(CONNECTION_BAD));
	}

	EXPECT_ANY_THROW((connection_checker::check(&api, pq_api_mock::test_connection), pq_api_mock::test_connection));
}

TEST(PostgresqlConnectionCheckerTest, TestSharedConnection)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, status(pq_api_mock::test_connection_shared.get())).WillOnce(testing::Return(CONNECTION_OK));

	EXPECT_EQ(connection_checker::check(&api, pq_api_mock::test_connection_shared), pq_api_mock::test_connection_shared.get());
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
