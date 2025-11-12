//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <zoo/spider/pathspec.h>
#include <zoo/spider/path.h>

namespace zoo {
namespace spider {

using segment = path_spec::segment;

TEST(PathSpecTest, TestDefaultConstructor)
{
	path_spec p{};
	EXPECT_THAT(p.segments(), testing::IsEmpty());
}

TEST(PathSpecTest, TestConstructorFromSegments)
{
	std::vector<segment> segments{};
	segments.push_back(segment{ "a", true });

	path_spec p{ segments };
	EXPECT_THAT(p.segments(), testing::Eq(segments));
}

TEST(PathSpecTest, TestConstructorFromString)
{
	EXPECT_THAT(path_spec{ "" }.segments(), testing::IsEmpty());
	EXPECT_THAT(path_spec{ "a" }.segments(), testing::ElementsAre(segment{ "a", false }));
	EXPECT_THAT(path_spec{ "{a}" }.segments(), testing::ElementsAre(segment{ "a", true }));
	EXPECT_THAT(path_spec{ "/a/{b}" }.segments(), testing::ElementsAre(segment{ "a", false }, segment{ "b", true }));
	EXPECT_THAT(path_spec{ "/a/{b}/c" }.segments(),
	            testing::ElementsAre(segment{ "a", false }, segment{ "b", true }, segment{ "c", false }));
	EXPECT_THAT(path_spec{ "a/{b}/c" }.segments(),
	            testing::ElementsAre(segment{ "a", false }, segment{ "b", true }, segment{ "c", false }));
	EXPECT_THAT(path_spec{ "/a/{b}/c/" }.segments(),
	            testing::ElementsAre(segment{ "a", false }, segment{ "b", true }, segment{ "c", false }));
}

TEST(PathSpecTest, TestEquality)
{
	EXPECT_EQ(path_spec{}, path_spec{});

	EXPECT_EQ((path_spec{ std::vector<segment>{ { "a", true } } }), (path_spec{ std::vector<segment>{ { "a", true } } }));
	EXPECT_NE((path_spec{ std::vector<segment>{ { "b", true } } }), (path_spec{ std::vector<segment>{ { "a", true } } }));
	EXPECT_NE((path_spec{ std::vector<segment>{ { "a", true } } }), (path_spec{ std::vector<segment>{ { "a", false } } }));
}

TEST(PathSpecTest, TestDivisionWithSegment)
{
	EXPECT_THAT((path_spec{ "a" } / segment{ "b", true }).segments(), testing::ElementsAre(segment{ "a", false }, segment{ "b", true }));
}

TEST(PathSpecTest, TestDivisionWithPathSpec)
{
	EXPECT_THAT((path_spec{ "a/b" } / path_spec{ "{c}/d" }).segments(),
	            testing::ElementsAre(segment{ "a", false }, segment{ "b", false }, segment{ "c", true }, segment{ "d", false }));
}

TEST(PathSpecTest, TestDivisionWithString)
{
	EXPECT_THAT((path_spec{ "a/b" } / "{c}/d").segments(),
	            testing::ElementsAre(segment{ "a", false }, segment{ "b", false }, segment{ "c", true }, segment{ "d", false }));
}

TEST(PathSpecTest, TestToString)
{
	EXPECT_THAT(path_spec{ "a/{b}/c" }.to_string(), testing::Eq("a/{b}/c"));
	EXPECT_THAT(path_spec{ "/a/{b}/c/" }.to_string(), testing::Eq("a/{b}/c"));
}

TEST(PathSpecTest, TestMatch)
{
	EXPECT_THAT(path_spec{ "a/b/" }.match(path{ "a/b" }).has_value(), testing::Eq(true));
	EXPECT_THAT(path_spec{ "a/b/c" }.match(path{ "a/b" }).has_value(), testing::Eq(false));
	EXPECT_THAT(path_spec{ "a/b" }.match(path{ "a/b/c" }).has_value(), testing::Eq(false));

	EXPECT_THAT(path_spec{ "a/b/{c}" }.match(path{ "a/b/x" }).has_value(), testing::Eq(true));
	EXPECT_THAT(path_spec{ "a/b/{c}" }.match(path{ "a/b/x" }).value().at("c"), testing::Eq("x"));
	EXPECT_THAT(path_spec{ "a/{b}/{c}" }.match(path{ "a/x/y" }).value().at("b"), testing::Eq("x"));
	EXPECT_THAT(path_spec{ "a/{b}/{c}" }.match(path{ "a/x/y" }).value().at("c"), testing::Eq("y"));
}

} // namespace spider
} // namespace zoo
