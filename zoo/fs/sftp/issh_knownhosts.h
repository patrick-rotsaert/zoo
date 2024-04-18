//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"
#include <string>

namespace zoo {
namespace fs {
namespace sftp {

class ZOO_EXPORT issh_known_hosts
{
public:
	enum class result
	{
		KNOWN,
		UNKNOWN,
		CHANGED
	};

public:
	virtual ~issh_known_hosts() noexcept;

	virtual result verify(const std::string& host, const std::string& pubkey_hash)  = 0;
	virtual void   persist(const std::string& host, const std::string& pubkey_hash) = 0;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
