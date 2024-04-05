#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.19)
project(zoo VERSION 0.1.0 LANGUAGES CXX)

# Make cache variables for install destinations
include(GNUInstallDirs)

enable_testing()

include(cmake/vars.cmake)

option(ZOO_BUILD_EXAMPLES "Build the examples." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_TEST "Build the tests." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_RUN_UNIT_TESTS_ON_BUILD "Run the unit tests during build." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_INSTALL "Include install rules for ${PROJECT_NAME}." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_CPACK "Include cpack rules for ${PROJECT_NAME}. Implies ZOO_INSTALL." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_USE_SPDLOG "Use spdlog as the default logger in the ${PROJECT_NAME} library" "${ZOO_IS_TOP_LEVEL}")
option(ZOO_THREAD_SAFE "Compile with locking code" ON)
option(ZOO_FETCH_DEPS "Fetch dependencies if not found using find_package" OFF)
option(ZOO_TREAT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ZOO_SQUID_WITH_POSTGRESQL "Include Squid PostgreSQL backend" ON)
option(ZOO_SQUID_WITH_MYSQL "Include Squid MySQL backend" ON)
option(ZOO_SQUID_WITH_SQLITE3 "Include Squid SQLite3 backend" ON)

if(ZOO_CPACK)
	set(ZOO_INSTALL ON)
endif()

# Let ZOO_SHARED_LIBS override BUILD_SHARED_LIBS
if(DEFINED ZOO_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${ZOO_SHARED_LIBS}")
endif()

include(cmake/project.cmake)

# Threads
if(ZOO_THREAD_SAFE)
	project_find_package(Threads REQUIRED)
	list(APPEND EXTRA_PUBLIC_LIBRARIES Threads::Threads)
endif()

add_subdirectory(zoo)

# if(ZOO_INSTALL)
# 	add_subdirectory(install)
# 	if(ZOO_CPACK)
# 		add_subdirectory(cpack)
# 	endif()
# endif()
