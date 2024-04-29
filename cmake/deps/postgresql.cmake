#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/options.cmake)

if(ZOO_SQUID_WITH_POSTGRESQL)
	# If this project is included as a subdirectory, the PostgreSQL::PostgreSQL target may already be defined.
	if(NOT TARGET PostgreSQL::PostgreSQL)
		project_find_package(PostgreSQL REQUIRED)
	endif()
endif()
