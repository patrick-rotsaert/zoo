#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/vars.cmake)

option(ZOO_BUILD_EXAMPLES "Build the examples." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_TEST "Build the tests." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_RUN_UNIT_TESTS_ON_BUILD "Run the unit tests during build." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_INSTALL "Include install rules for zoo." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_CPACK "Include cpack rules for zoo. Implies ZOO_INSTALL." "${ZOO_IS_TOP_LEVEL}")
option(ZOO_THREAD_SAFE "Compile with locking code" ON)
option(ZOO_TREAT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ZOO_WITH_BITCASK "Build bitcask library" "${ZOO_IS_TOP_LEVEL}")
option(ZOO_WITH_FS "Build fs library" "${ZOO_IS_TOP_LEVEL}")
option(ZOO_WITH_SPIDER "Build spider library" "${ZOO_IS_TOP_LEVEL}")
option(ZOO_WITH_SQUID "Build squid library" "${ZOO_IS_TOP_LEVEL}")
option(ZOO_SQUID_WITH_POSTGRESQL "Include Squid PostgreSQL backend" ON)
option(ZOO_SQUID_WITH_MYSQL "Include Squid MySQL backend" ON)
option(ZOO_SQUID_WITH_SQLITE3 "Include Squid SQLite3 backend" ON)

# Logging verbosity of the library
set(LOGGING_LEVELS trace debug info warn err critical off) # Values are as in zoo/common/logging/log_level.h
if(ZOO_DEBUG_BUILD)
	set(DEFAULT_LOGGING_LEVEL trace)
else()
	set(DEFAULT_LOGGING_LEVEL warn)
endif()
set(ZOO_LOGGING_LEVEL ${DEFAULT_LOGGING_LEVEL} CACHE STRING "Logging verbosity of the ${PROJECT_NAME} library")
set_property(CACHE ZOO_LOGGING_LEVEL PROPERTY STRINGS ${LOGGING_LEVELS})

if(ZOO_CPACK)
	set(ZOO_INSTALL ON)
endif()
