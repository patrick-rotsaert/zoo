//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/squid/sqlite3/connection.h"
#include "zoo/squid/core/statement.h"

#include <boost/serialization/nvp.hpp>

#include <cstdint>

// Regression tests for basic_statement::bind_results: arguments following a result
// that is bound via a bind() method or Boost.Serialization used to be silently dropped.

namespace zoo {
namespace squid {
namespace sqlite {
namespace {

struct ab_row final
{
	std::int32_t a{};
	std::int32_t b{};

	template<class Binder>
	void bind(Binder& binder)
	{
		binder.bind("a", a);
		binder.bind("b", b);
	}
};

struct cd_row final
{
	std::int32_t c{};
	std::int32_t d{};

	template<class Binder>
	void bind(Binder& binder)
	{
		binder.bind("c", c);
		binder.bind("d", d);
	}
};

struct ef_row final
{
	std::int32_t e{};
	std::int32_t f{};

	template<class Archive>
	void serialize(Archive& ar, const unsigned int /*version*/)
	{
		ar& BOOST_SERIALIZATION_NVP(e) & BOOST_SERIALIZATION_NVP(f);
	}
};

struct gh_row final
{
	std::int32_t g{};
	std::int32_t h{};

	template<class Archive>
	void serialize(Archive& ar, const unsigned int /*version*/)
	{
		ar& BOOST_SERIALIZATION_NVP(g) & BOOST_SERIALIZATION_NVP(h);
	}
};

} // namespace

TEST(BindResults, BindMethodStructDoesNotDropTrailingArgs)
{
	connection conn{ ":memory:" };

	ab_row ab;
	cd_row cd;

	statement st{ conn, "SELECT 1 AS a, 2 AS b, 3 AS c, 4 AS d" };
	st.bind_results(ab, cd); // cd follows a bind()-method struct and must not be dropped
	st.execute();

	ASSERT_TRUE(st.fetch());
	EXPECT_EQ(ab.a, 1);
	EXPECT_EQ(ab.b, 2);
	EXPECT_EQ(cd.c, 3);
	EXPECT_EQ(cd.d, 4);
}

TEST(BindResults, SerializableStructDoesNotDropTrailingArgs)
{
	connection conn{ ":memory:" };

	ef_row ef;
	gh_row gh;

	statement st{ conn, "SELECT 1 AS e, 2 AS f, 3 AS g, 4 AS h" };
	st.bind_results(ef, gh); // gh follows a Boost-serializable struct and must not be dropped
	st.execute();

	ASSERT_TRUE(st.fetch());
	EXPECT_EQ(ef.e, 1);
	EXPECT_EQ(ef.f, 2);
	EXPECT_EQ(gh.g, 3);
	EXPECT_EQ(gh.h, 4);
}

} // namespace sqlite
} // namespace squid
} // namespace zoo
