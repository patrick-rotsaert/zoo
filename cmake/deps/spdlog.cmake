#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/policies.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/vars.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/deps/fmtlib.cmake)

# If this project is included as source, the spdlog::spdlog target may already be defined.
if(NOT TARGET spdlog::spdlog)
	# Prefer existing package
	find_package(spdlog QUIET)
	if(spdlog_FOUND)
		project_add_dependency(spdlog REQUIRED)
	elseif(ZOO_FETCH_DEPS)
		include(FetchContent)
		set(SPDLOG_FMT_EXTERNAL ON)
		set(SPDLOG_INSTALL "${ZOO_INSTALL}")
		FetchContent_Declare(spdlog
			GIT_REPOSITORY https://github.com/gabime/spdlog.git
			GIT_TAG v1.13.0
		)
		FetchContent_MakeAvailable(spdlog)
	else()
		message(FATAL_ERROR "Could not find package spdlog and fetching it was disabled by ZOO_FETCH_DEPS")
	endif()
endif()
