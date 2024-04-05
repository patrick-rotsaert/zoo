//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/common/logging/logging.h"
#include "zoo/common/logging/spdlog_backend.h"

namespace zoo {
namespace logging {

std::unique_ptr<ibackend> logging::backend = std::make_unique<spdlog_backend>();

void logging::set_backend(std::unique_ptr<ibackend>&& backend)
{
	logging::backend = std::move(backend);
}

} // namespace logging
} // namespace zoo
