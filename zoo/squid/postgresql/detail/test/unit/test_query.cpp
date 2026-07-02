//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>
#include <zoo/squid/postgresql/detail/query.h>

namespace zoo {
namespace squid {
namespace postgresql {

TEST(PostgresqlQueryTest, EmptyString)
{
	postgresql_query q{ "" };
	EXPECT_EQ(q.query(), "");
	EXPECT_EQ(q.parameter_count(), 0);
}

TEST(PostgresqlQueryTest, OneParameter)
{
	postgresql_query q{ "SELECT :first" };
	EXPECT_EQ(q.query(), "SELECT $1");
	EXPECT_EQ(q.parameter_count(), 1);
	EXPECT_EQ(q.parameter_name_pos_map().size(), 1u);
	auto map = q.parameter_name_pos_map();
	EXPECT_EQ(map["first"], 1);
}

TEST(PostgresqlQueryTest, TwoParameters)
{
	postgresql_query q{ "SELECT :first, :second" };
	EXPECT_EQ(q.query(), "SELECT $1, $2");
	EXPECT_EQ(q.parameter_count(), 2);
	EXPECT_EQ(q.parameter_name_pos_map().size(), 2u);
	auto map = q.parameter_name_pos_map();
	EXPECT_EQ(map["first"], 1);
	EXPECT_EQ(map["second"], 2);
}

TEST(PostgresqlQueryTest, ParametersWithSameName)
{
	postgresql_query q{ "SELECT :first, :second, :first" };
	EXPECT_EQ(q.query(), "SELECT $1, $2, $1");
	EXPECT_EQ(q.parameter_count(), 2);
	EXPECT_EQ(q.parameter_name_pos_map().size(), 2u);
	auto map = q.parameter_name_pos_map();
	EXPECT_EQ(map["first"], 1);
	EXPECT_EQ(map["second"], 2);
}

TEST(PostgresqlQueryTest, ParametersWithSameNameBegin)
{
	postgresql_query q{ "SELECT :first, :second, :first_not_same" };
	EXPECT_EQ(q.query(), "SELECT $1, $2, $3");
	EXPECT_EQ(q.parameter_count(), 3);
	EXPECT_EQ(q.parameter_name_pos_map().size(), 3u);
	auto map = q.parameter_name_pos_map();
	EXPECT_EQ(map["first"], 1);
	EXPECT_EQ(map["second"], 2);
	EXPECT_EQ(map["first_not_same"], 3);
}

TEST(PostgresqlQueryTest, ParametersWithAlternateSyntax)
{
	postgresql_query q{ "SELECT :first, $second, @first FROM foo" };
	EXPECT_EQ(q.query(), "SELECT $1, $2, $1 FROM foo");
	EXPECT_EQ(q.parameter_count(), 2);
	EXPECT_EQ(q.parameter_name_pos_map().size(), 2u);
	auto map = q.parameter_name_pos_map();
	EXPECT_EQ(map["first"], 1);
	EXPECT_EQ(map["second"], 2);
}

TEST(PostgresqlQueryTest, ParameterInStringLiteral)
{
	postgresql_query q{ "SELECT :first, :second, ':third'" };
	EXPECT_EQ(q.query(), "SELECT $1, $2, ':third'");
	EXPECT_EQ(q.parameter_count(), 2);
	EXPECT_EQ(q.parameter_name_pos_map().size(), 2u);
	auto map = q.parameter_name_pos_map();
	EXPECT_EQ(map["first"], 1);
	EXPECT_EQ(map["second"], 2);
}

TEST(PostgresqlQueryTest, ParameterInIdentifier)
{
	postgresql_query q{ "SELECT :first, :second, \":third\"" };
	EXPECT_EQ(q.query(), "SELECT $1, $2, \":third\"");
	EXPECT_EQ(q.parameter_count(), 2);
	EXPECT_EQ(q.parameter_name_pos_map().size(), 2u);
	auto map = q.parameter_name_pos_map();
	EXPECT_EQ(map["first"], 1);
	EXPECT_EQ(map["second"], 2);
}

TEST(PostgresqlQueryTest, CastingOperator)
{
	{
		postgresql_query q{ "SELECT :first::xxx, :second" };
		EXPECT_EQ(q.query(), "SELECT $1::xxx, $2");
		EXPECT_EQ(q.parameter_count(), 2);
		EXPECT_EQ(q.parameter_name_pos_map().size(), 2u);
		auto map = q.parameter_name_pos_map();
		EXPECT_EQ(map["first"], 1);
		EXPECT_EQ(map["second"], 2);
	}
	{
		postgresql_query q{ "SELECT :first :: xxx, :second" };
		EXPECT_EQ(q.query(), "SELECT $1 :: xxx, $2");
		EXPECT_EQ(q.parameter_count(), 2);
		EXPECT_EQ(q.parameter_name_pos_map().size(), 2u);
		auto map = q.parameter_name_pos_map();
		EXPECT_EQ(map["first"], 1);
		EXPECT_EQ(map["second"], 2);
	}
}

TEST(PostgresqlQueryTest, ParameterNamesMayContainDigits)
{
	postgresql_query q{ "SELECT :param1, :a2b" };
	EXPECT_EQ(q.query(), "SELECT $1, $2");
	EXPECT_EQ(q.parameter_count(), 2);
	auto map = q.parameter_name_pos_map();
	EXPECT_EQ(map["param1"], 1);
	EXPECT_EQ(map["a2b"], 2);
}

TEST(PostgresqlQueryTest, DigitImmediatelyAfterSigilIsNotAParameter)
{
	// A name must start with a letter or underscore, so ":1" is a literal, not a
	// parameter. This also keeps PostgreSQL positional placeholders like "$1" intact.
	postgresql_query q{ "SELECT :1, $1" };
	EXPECT_EQ(q.query(), "SELECT :1, $1");
	EXPECT_EQ(q.parameter_count(), 0);
}

TEST(PostgresqlQueryTest, NonAsciiBytesAreHandledWithoutUndefinedBehaviour)
{
	// High (non-ASCII) bytes must not be passed to std::isalpha/isalnum as a negative
	// char. They are not name characters, so the text round-trips and yields no parameters.
	postgresql_query q{ "SELECT '\xC3\xA9' AS x, :\xC3\xA9" };
	EXPECT_EQ(q.query(), "SELECT '\xC3\xA9' AS x, :\xC3\xA9");
	EXPECT_EQ(q.parameter_count(), 0);
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
