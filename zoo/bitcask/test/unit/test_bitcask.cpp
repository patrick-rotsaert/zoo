//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>
#include <zoo/bitcask/bitcask.h>
#include <filesystem>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/directory.hpp>

namespace zoo {
namespace bitcask {

class BitcaskTests : public testing::Test
{
	std::filesystem::path dir_;

protected:
	BitcaskTests()
	    : dir_{
		    (boost::filesystem::unique_path(boost::filesystem::temp_directory_path() / "bitcask_test_suite-%%%%-%%%%-%%%%-%%%%")).string()
	    }
	{
	}

	void SetUp() override
	{
		std::filesystem::create_directories(this->dir_);
	}

	void TearDown() override
	{
		std::filesystem::remove_all(this->dir_);
	}

	const std::filesystem::path& dir() const
	{
		return this->dir_;
	}
};

namespace {

using map_type = std::map<key_type, value_type>;

map_type load_map(bitcask& bc)
{
	auto map = map_type{};

	bc.traverse([&](const auto& key, const auto& value) {
		map[std::string{ key }] = value;
		return true;
	});

	return map;
}

} // namespace

TEST_F(BitcaskTests, basic_tests)
{
	bitcask bc{ this->dir() };
	EXPECT_TRUE(bc.empty());
	EXPECT_FALSE(bc.get("key_a").has_value());
	EXPECT_FALSE(bc.del("key_a"));
	EXPECT_TRUE(bc.put("key_a", "value_a")); // insert
	EXPECT_TRUE(bc.get("key_a").has_value());
	EXPECT_EQ(bc.get("key_a").value(), "value_a");
	EXPECT_FALSE(bc.empty());
	EXPECT_FALSE(bc.put("key_a", "value_a_2")); // update
	EXPECT_TRUE(bc.get("key_a").has_value());
	EXPECT_EQ(bc.get("key_a").value(), "value_a_2");
	EXPECT_FALSE(bc.empty());
	EXPECT_TRUE(bc.del("key_a"));
	EXPECT_TRUE(bc.empty());
	EXPECT_FALSE(bc.get("key_a").has_value());
	EXPECT_TRUE(bc.empty());
}

TEST_F(BitcaskTests, traverse_tests)
{
	bitcask bc{ this->dir() };
	auto    map = map_type{};

	EXPECT_EQ(map, load_map(bc));

	map["key_a"] = "value_a";
	map["key_b"] = "value_b";
	map["key_c"] = "value_c";
	EXPECT_TRUE(bc.put("key_a", "value_a")); // insert
	EXPECT_TRUE(bc.put("key_b", "value_b")); // insert
	EXPECT_TRUE(bc.put("key_c", "value_c")); // insert
	EXPECT_EQ(map, load_map(bc));

	map["key_b"] = "value_b_2";
	EXPECT_FALSE(bc.put("key_b", "value_b_2")); // update
	EXPECT_EQ(map, load_map(bc));

	map.erase("key_a");
	EXPECT_TRUE(bc.del("key_a"));
	EXPECT_EQ(map, load_map(bc));
}

TEST_F(BitcaskTests, test_close_and_reopen)
{
	auto map = map_type{};

	{
		bitcask bc{ this->dir() };

		map["key_a"] = "value_a";
		map["key_b"] = "value_b";
		map["key_c"] = "value_c";
		EXPECT_TRUE(bc.put("key_a", "value_a")); // insert
		EXPECT_TRUE(bc.put("key_b", "value_b")); // insert
		EXPECT_TRUE(bc.put("key_c", "value_c")); // insert
		EXPECT_EQ(map, load_map(bc));

		map["key_b"] = "value_b_2";
		EXPECT_FALSE(bc.put("key_b", "value_b_2")); // update
		EXPECT_EQ(map, load_map(bc));

		map.erase("key_a");
		EXPECT_TRUE(bc.del("key_a"));
		EXPECT_EQ(map, load_map(bc));
	}

	{
		bitcask bc{ this->dir() };
		EXPECT_EQ(map, load_map(bc));
	}
}

TEST_F(BitcaskTests, test_merge)
{
	auto map = map_type{};

	{
		const std::string value(512u, 'X');
		map["key_a"] = value;

		bitcask bc{ this->dir() };

		// Limit the size of data files to 1Kb.
		// Data files will now be closed sooner.
		bc.max_file_size(1024);

		// Updating (or deleting) a key appends a new record to the active data file.
		// In time this will take up lots of unused disk space.
		for (auto n = 100; n; --n)
		{
			bc.put("key_a", value);
		}

		// After the 100 records logged, there will be 50 data files.
		// Free up disk space by compacting (merging) the immutable data files.
		bc.merge();

		EXPECT_EQ(map, load_map(bc));
	}

	{
		bitcask bc{ this->dir() };
		EXPECT_EQ(map, load_map(bc));
	}
}

TEST_F(BitcaskTests, test_clear)
{
	{
		bitcask bc{ this->dir() };
		EXPECT_TRUE(bc.put("key_a", "value_a")); // insert
		EXPECT_FALSE(bc.empty());
	}

	bitcask::clear(this->dir());

	bitcask bc{ this->dir() };
	EXPECT_TRUE(bc.empty());
	EXPECT_EQ(map_type{}, load_map(bc));
}

} // namespace bitcask
} // namespace zoo
