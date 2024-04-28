//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/common/logging/logging.h"
#include "zoo/common/logging/spdlog_backend.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4458)
#endif

namespace zoo {
namespace logging {

ZOO_COMMON_API std::unique_ptr<ibackend> logging::backend = std::make_unique<spdlog_backend>();

void logging::set_backend(std::unique_ptr<ibackend>&& backend)
{
	logging::backend = std::move(backend);
}

} // namespace logging
} // namespace zoo
