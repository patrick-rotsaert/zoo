//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/bitcask/bitcask.h"
#include <iostream>
#include <filesystem>
#include <cassert>

using namespace zoo::bitcask;

void basic_usage()
{
	// Open or create a bitcask.
	// Constructor takes a path to a directory.
	// The directory is created recursively if it does not exist.
	const auto dir = std::filesystem::temp_directory_path() / "bitcask" / "bcdir0001";
	bitcask    bc{ dir };

	// Only one process/thread can have a bitbask open.
	// bitcask other{ dir }; // would throw, since `dir` is locked.

	// Put (upsert) a k/v pair.
	bc.put("key01", "the value for key01");

	// Update a k/v pair.
	bc.put("key01", "the new value for key01");

	// Get a value, returns an std::optional.
	const auto value = bc.get("key01");
	assert(value.has_value() && value.value() == "the new value for key01");

	// Delete a k/v pair.
	bc.del("key01");
	assert(!bc.get("key01").has_value());
}

void merge()
{
	const auto dir = std::filesystem::temp_directory_path() / "bitcask" / "bcdir0001";
	bitcask    bc{ dir };

	// Limit the size of data files to 1Mb.
	// Data files will now be closed sooner.
	bc.max_file_size(1 * 1024 * 1024);

	// Updating (or deleting) a key appends a new record to the active data file.
	// In time this will take up lots of unused disk space.
	for (auto n = 100000; n; --n)
	{
		bc.put("The key", "The value for the key");
	}

	// After the 100K records logged, there will be 5 data files, 4 of which
	// are ~1Mb in size.
	// Free up disk space by compacting (merging) the immutable data files.
	bc.merge();

	// There is now only one data file left (the active file).
}

int main()
{
	try
	{
		basic_usage();
		merge();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return 1;
	}
}
