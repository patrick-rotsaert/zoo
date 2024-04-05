//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/bitcask/basictypes.h"
#include "zoo/common/api.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <functional>

namespace zoo {
namespace bitcask {

class ZOO_EXPORT bitcask final
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

	bool empty() const;

	std::optional<value_type> get(const std::string_view& key);

	/// Returns true if the key was inserted, false if the key existed.
	bool put(const std::string_view& key, const std::string_view& value);

	/// Returns true if the key was deleted, false if the key did not exist.
	bool del(const std::string_view& key);

	bool traverse(std::function<bool(const std::string_view& key, const std::string_view& value)> callback);

	// maintenance
	void merge();

	// destruction
	// make sure no bitcask instance exists with this directory!
	static void clear(const std::filesystem::path& directory);
};

} // namespace bitcask
}
