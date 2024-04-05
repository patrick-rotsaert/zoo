//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/bitcask/keydir.h"
#include "zoo/bitcask/basictypes.h"

#include <filesystem>
#include <memory>

namespace zoo {
namespace bitcask {

class datadir final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit datadir(const std::filesystem::path& directory);
	~datadir() noexcept;

	datadir(datadir&&)            = default;
	datadir& operator=(datadir&&) = default;

	datadir(const datadir&)            = delete;
	datadir& operator=(const datadir&) = delete;

	off64_t max_file_size() const;
	void    max_file_size(off64_t size);

	void build_keydir(keydir& kd);

	value_type   get(const keydir::info& info);
	keydir::info put(const std::string_view& key, const std::string_view& value, version_type version);
	void         del(const std::string_view& key, version_type version);

	// maintenance
	void merge(keydir& kd);

	// destruction
	// make sure no datadir instance exists with this directory!
	static void clear(const std::filesystem::path& directory);
};

} // namespace bitcask
} // namespace zoo
