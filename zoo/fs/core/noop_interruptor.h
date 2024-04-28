//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/config.h"
#include "zoo/fs/core/iinterruptor.h"

namespace zoo {
namespace fs {

class ZOO_FS_CORE_API noop_interruptor final : public iinterruptor
{
	void interrupt() override;
	bool is_interrupted() override;
	bool wait_for_interruption(std::chrono::milliseconds duration) override;
};

} // namespace fs
} // namespace zoo
