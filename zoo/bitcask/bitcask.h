//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/bitcask/basictypes.h"
#include "zoo/bitcask/config.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <functional>

namespace zoo {
namespace bitcask {

class ZOO_BITCASK_API bitcask final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit bitcask(const std::filesystem::path& directory);
	~bitcask() noexcept;

	bitcask(bitcask&&)            = default;
	bitcask& operator=(bitcask&&) = default;

	bitcask(const bitcask&)            = delete;
	bitcask& operator=(const bitcask&) = delete;

	off64_t max_file_size() const;
	void    max_file_size(off64_t size);

	// Returns true if the bitcask does not contain any keys.
	bool empty() const;

	/// Get a key-value pair.
	/// Returns the value or std::nullopt if the key does not exist.
	std::optional<value_type> get(const std::string_view& key);

	/// Insert or update a key-value pair.
	/// Returns true if the key was inserted, false if the key existed.
	bool put(const std::string_view& key, const std::string_view& value);

	/// Delete a key and its value.
	/// Returns true if the key was deleted, false if the key did not exist.
	bool del(const std::string_view& key);

	/// Iterate over all key-value pairs.
	/// Iteration will stop if the callback returns false.
	/// Returns true if all keys were traversed, false if a callback returned false.
	bool traverse(std::function<bool(const std::string_view& key, const std::string_view& value)> callback);

	// maintenance
	void merge();

	// destruction
	// make sure no bitcask instance exists with this directory!
	static void clear(const std::filesystem::path& directory);
};

} // namespace bitcask
} // namespace zoo
