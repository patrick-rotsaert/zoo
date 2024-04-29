#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

# If this project is included as source, the spdlog::spdlog target may already be defined.
if(NOT TARGET spdlog::spdlog)
	project_find_package(spdlog CONFIG REQUIRED)
endif()
