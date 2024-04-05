//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/ssh_identity.h"
#include "zoo/common/api.h"
#include <memory>
#include <vector>

namespace zoo {
namespace fs {
namespace sftp {

class ZOO_EXPORT issh_identity_factory
{
public:
	virtual ~issh_identity_factory() noexcept;

	virtual std::vector<std::shared_ptr<ssh_identity>> create() = 0;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
