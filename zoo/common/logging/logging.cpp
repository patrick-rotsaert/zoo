//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/common/logging/logging.h"
#include "zoo/common/config.h"
#ifdef ZOO_USE_SPDLOG
#include "zoo/common/logging/spdlog_backend.h"
#endif

namespace zoo {
namespace logging {

#ifdef ZOO_USE_SPDLOG
std::unique_ptr<ibackend> logging::backend = std::make_unique<spdlog_backend>();
#else
std::unique_ptr<ibackend> logging::backend{};
#endif

void logging::set_backend(std::unique_ptr<ibackend>&& backend)
{
	logging::backend = std::move(backend);
}

} // namespace logging
} // namespace zoo
