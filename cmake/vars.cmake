#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" ZOO_IS_TOP_LEVEL)

if(NOT CMAKE_BUILD_TYPE)
	message(FATAL_ERROR "CMAKE_BUILD_TYPE must be set")
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LC)

if(CMAKE_BUILD_TYPE_LC MATCHES "^debug")
	set(ZOO_DEBUG_BUILD 1)
else()
	set(ZOO_DEBUG_BUILD 0)
endif()

# Let ZOO_SHARED_LIBS override BUILD_SHARED_LIBS
if(DEFINED ZOO_SHARED_LIBS)
	set(BUILD_SHARED_LIBS "${ZOO_SHARED_LIBS}")
endif()
