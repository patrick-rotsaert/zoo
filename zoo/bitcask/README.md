# BITCASK - C++ Bitcask Implementation

Bitcask implementation, based on the [Bitcask paper](https://riak.com/assets/bitcask-intro.pdf).

For parallel use from multiple threads, make sure the library is compiled with the CMake option
```
ZOO_THREAD_SAFE=ON
```

## Quick start

All code samples imply:
```cpp
using namespace zoo::bitcask;
```

### Basic usage

```cpp
#include "zoo/bitcask/bitcask.h"

void basic_usage()
{
	// Open or create a bitcask.
	// Constructor takes a path to a directory.
	// The directory is created recursively if it does not exist.
	const auto dir = std::filesystem::temp_directory_path() / "bitcask" / "bcdir0001";
	bitcask bc{ dir };

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
```

### Merging

```cpp
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
```

## Motivation

Stumbling across the [Bitcask paper](https://riak.com/assets/bitcask-intro.pdf), I thought this would be a fun weekend project.
Indeed fun it was, but it took a little longer than a weekend to finish.
Especially the merging was challenging to implement.
The paper is quite vague on that part and I wanted to make it as efficient as possible while still allowing CRUD operations to run in parallel.

There do exist good implementations already, but I intentionally did not look at them. Considered this to be an excercise.
