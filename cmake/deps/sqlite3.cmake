#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/options.cmake)

if(ZOO_SQUID_WITH_SQLITE3)
	# If this project is included as a subdirectory, the MySQL::MySQL target may already be defined.
	if(NOT TARGET SQLite::SQLite3)
		project_find_package(unofficial-sqlite3 CONFIG QUIET)
		if(unofficial-sqlite3_FOUND)
			add_library(SQLite::SQLite3 ALIAS unofficial::sqlite3::sqlite3)
		else()
			project_find_package(SQLite3 REQUIRED)
		endif()
	endif()
endif()
