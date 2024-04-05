//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/bitcask/bitcask.h"
#include "zoo/bitcask/datadir.h"
#include "zoo/bitcask/keydir.h"

namespace zoo {
namespace bitcask {

class bitcask::impl final
{
	datadir datadir_;
	keydir  keydir_;

public:
	explicit impl(const std::filesystem::path& directory)
	    : datadir_{ directory }
	    , keydir_{}
	{
		this->datadir_.build_keydir(this->keydir_);
	}

	off64_t max_file_size() const
	{
		return this->datadir_.max_file_size();
	}

	void max_file_size(off64_t size)
	{
		return this->datadir_.max_file_size(size);
	}

	bool empty() const
	{
		return this->keydir_.empty();
	}

	std::optional<value_type> get(const std::string_view& key)
	{
		const auto info = this->keydir_.get(key);
		if (info)
		{
			return this->datadir_.get(info.value());
		}
		else
		{
			return std::nullopt;
		}
	}

	bool put(const std::string_view& key, const std::string_view& value)
	{
		return this->keydir_.put(key, this->datadir_.put(key, value, this->keydir_.next_version()));
	}

	bool del(const std::string_view& key)
	{
		this->datadir_.del(key, this->keydir_.next_version());
		return this->keydir_.del(key);
	}

	bool traverse(std::function<bool(const std::string_view& key, const std::string_view& value)> callback)
	{
		return this->keydir_.traverse([&](const auto& key, const auto& info) { return callback(key, this->datadir_.get(info)); });
	}

	void merge()
	{
		return this->datadir_.merge(this->keydir_);
	}

	static void clear(const std::filesystem::path& directory)
	{
		datadir::clear(directory);
	}
};

bitcask::bitcask(const std::filesystem::path& directory)
    : pimpl_{ std::make_unique<impl>(directory) }
{
}

bitcask::~bitcask() noexcept
{
}

off64_t bitcask::max_file_size() const
{
	return this->pimpl_->max_file_size();
}

void bitcask::max_file_size(off64_t size)
{
	return this->pimpl_->max_file_size(size);
}

bool bitcask::empty() const
{
	return this->pimpl_->empty();
}

std::optional<value_type> bitcask::get(const std::string_view& key)
{
	return this->pimpl_->get(key);
}

bool bitcask::put(const std::string_view& key, const std::string_view& value)
{
	return this->pimpl_->put(key, value);
}

bool bitcask::del(const std::string_view& key)
{
	return this->pimpl_->del(key);
}

bool bitcask::traverse(std::function<bool(const std::string_view&, const std::string_view&)> callback)
{
	return this->pimpl_->traverse(callback);
}

void bitcask::merge()
{
	return this->pimpl_->merge();
}

void bitcask::clear(const std::filesystem::__cxx11::path& directory)
{
	impl::clear(directory);
}

} // namespace bitcask
} // namespace zoo
