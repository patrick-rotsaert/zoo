#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/policies.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/vars.cmake)

# If this project is included as a subdirectory, the fmt::fmt target may already be defined.
if(NOT TARGET fmt::fmt)
	# Prefer existing package
	find_package(fmt QUIET)
	if(fmt_FOUND)
		project_add_dependency(fmt REQUIRED)
	elseif(ZOO_FETCH_DEPS)
		include(FetchContent)
		set(FMT_INSTALL "${ZOO_INSTALL}")
		FetchContent_Declare(fmt
			GIT_REPOSITORY https://github.com/fmtlib/fmt.git
			GIT_TAG 10.0.0
		)
		FetchContent_MakeAvailable(fmt)
	else()
		message(FATAL_ERROR "Could not find package fmt and fetching it was disabled by ZOO_FETCH_DEPS")
	endif()
endif()
