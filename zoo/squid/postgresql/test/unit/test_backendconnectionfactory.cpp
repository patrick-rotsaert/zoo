//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>
#include <zoo/squid/postgresql/backendconnectionfactory.h>
#include <zoo/squid/postgresql/backendconnection.h>
#include <zoo/squid/postgresql/detail/pqapimock.h>

namespace zoo {
namespace squid {
namespace postgresql {
namespace {

static constexpr auto g_connection_info = "the connection info";

} // namespace

TEST(BackendConnectionFactoryTests, TestCreateBackendConnection)
{
	auto api = pq_api_mock_nice{};

	EXPECT_CALL(api, connectdb(testing::StrEq(g_connection_info))).WillOnce(testing::Return(pq_api_mock::test_connection));
	EXPECT_CALL(api, status(pq_api_mock::test_connection)).WillOnce(testing::Return(CONNECTION_OK));
	EXPECT_CALL(api, finish(pq_api_mock::test_connection)).Times(1);

	auto virtconn = backend_connection_factory{ api }.create_backend_connection(g_connection_info);
	auto conn     = std::dynamic_pointer_cast<backend_connection>(virtconn);
	EXPECT_NE(conn, nullptr);

	EXPECT_EQ(conn->native_connection().get(), pq_api_mock::test_connection);
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
