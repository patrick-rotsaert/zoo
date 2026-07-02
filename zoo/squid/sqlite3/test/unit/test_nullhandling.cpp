//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/squid/sqlite3/connection.h"
#include "zoo/squid/core/statement.h"

#include <optional>
#include <string>

// Regression tests for per-row NULL handling against a real in-memory database.
// SQLite is dynamically typed: a column's type (and NULL-ness) is per-row, so the
// backend must read the type for each fetched row rather than caching the first row's.

namespace zoo {
namespace squid {
namespace sqlite {
namespace {

TEST(QueryResultsNullHandling, NullOnLaterRow)
{
	connection conn{ ":memory:" };

	statement{ conn, "CREATE TABLE t (id INTEGER PRIMARY KEY, v TEXT)" }.execute();
	statement{ conn, "INSERT INTO t (id, v) VALUES (1, 'hello'), (2, NULL), (3, 'world')" }.execute();

	statement st{ conn, "SELECT v FROM t ORDER BY id" };

	std::optional<std::string> v;
	st.bind_result(v);
	st.execute();

	// Row 1 is non-NULL.
	ASSERT_TRUE(st.fetch());
	ASSERT_TRUE(v.has_value());
	EXPECT_EQ(*v, "hello");

	// Row 2 is NULL: with the first row's type cached as TEXT this used to call
	// sqlite3_column_text on a NULL value and throw; it must reset the optional instead.
	ASSERT_TRUE(st.fetch());
	EXPECT_FALSE(v.has_value());

	// Row 3 is non-NULL again.
	ASSERT_TRUE(st.fetch());
	ASSERT_TRUE(v.has_value());
	EXPECT_EQ(*v, "world");

	EXPECT_FALSE(st.fetch());
}

TEST(QueryResultsNullHandling, NullOnFirstRow)
{
	connection conn{ ":memory:" };

	statement{ conn, "CREATE TABLE t (id INTEGER PRIMARY KEY, v TEXT)" }.execute();
	statement{ conn, "INSERT INTO t (id, v) VALUES (1, NULL), (2, 'world')" }.execute();

	statement st{ conn, "SELECT v FROM t ORDER BY id" };

	std::optional<std::string> v;
	st.bind_result(v);
	st.execute();

	// Row 1 is NULL.
	ASSERT_TRUE(st.fetch());
	EXPECT_FALSE(v.has_value());

	// Row 2 is non-NULL: with the first row's type cached as SQLITE_NULL this used to
	// be treated as NULL too (silent data loss); it must read the current row's type.
	ASSERT_TRUE(st.fetch());
	ASSERT_TRUE(v.has_value());
	EXPECT_EQ(*v, "world");

	EXPECT_FALSE(st.fetch());
}

} // namespace
} // namespace sqlite
} // namespace squid
} // namespace zoo
