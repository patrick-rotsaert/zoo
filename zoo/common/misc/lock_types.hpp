//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/config.h"

#ifdef ZOO_THREAD_SAFE
#include <mutex>
#include <shared_mutex>

namespace zoo {

using lock_type       = std::unique_lock<std::mutex>;
using write_lock_type = std::unique_lock<std::shared_mutex>;
using read_lock_type  = std::shared_lock<std::shared_mutex>;

class locker final
{
	std::mutex mutex_{};

public:
	lock_type lock()
	{
		return lock_type{ this->mutex_ };
	}
};

class shared_locker final
{
	std::shared_mutex mutex_{};

public:
	read_lock_type read_lock()
	{
		return read_lock_type{ this->mutex_ };
	}

	write_lock_type write_lock()
	{
		return write_lock_type{ this->mutex_ };
	}
};

} // namespace zoo

#else

namespace zoo {

struct lock_type final
{
	void unlock()
	{
	}
};
using write_lock_type = lock_type;
using read_lock_type  = lock_type;

class locker final
{
public:
	lock_type lock()
	{
		return lock_type{};
	}
};

class shared_locker final
{
public:
	read_lock_type read_lock()
	{
		return read_lock_type{};
	}

	write_lock_type write_lock()
	{
		return write_lock_type{};
	}
};

// namespace zoo

#endif
