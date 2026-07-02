//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/squid/sqlite3/connection.h"
#include "zoo/squid/core/statement.h"

#include <cstdint>

// Regression test for basic_statement::query(): streaming a new query with operator<< after a
// previous execute() must replace the query text, not append to it.

namespace zoo {
namespace squid {
namespace sqlite {
namespace {

TEST(StatementQueryStreaming, StreamingNewQueryReplacesPrevious)
{
	connection conn{ ":memory:" };

	statement{ conn, "CREATE TABLE t (v INTEGER)" }.execute();

	statement st{ conn };
	st << "INSERT INTO t (v) VALUES (1)";
	st.execute();

	// Reuse the same statement object for a different query.
	st << "INSERT INTO t (v) VALUES (2)";
	st.execute();

	statement    q{ conn, "SELECT SUM(v) FROM t" };
	std::int64_t sum{};
	q.bind_result(sum);
	q.execute();

	ASSERT_TRUE(q.fetch());
	// 1 + 2. If the second query had appended to the first, SQLite would prepare only the leading
	// statement and the second insert would be lost (sum would be 2).
	EXPECT_EQ(sum, 3);
}

} // namespace
} // namespace sqlite
} // namespace squid
} // namespace zoo
