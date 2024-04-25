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

if(ZOO_CPACK)
	set(ZOO_INSTALL ON)
endif()
