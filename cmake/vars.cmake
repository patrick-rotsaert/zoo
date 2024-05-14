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

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(zoo_ARCH x86)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(zoo_ARCH x64)
else()
	message(FATAL_ERROR "Unknown architecture")
endif()

if(CMAKE_BUILD_TYPE_LC MATCHES "^debug")
	set(ZOO_DEBUG_BUILD 1)
else()
	set(ZOO_DEBUG_BUILD 0)
endif()

# Let ZOO_SHARED_LIBS override BUILD_SHARED_LIBS
if(DEFINED ZOO_SHARED_LIBS)
	set(BUILD_SHARED_LIBS "${ZOO_SHARED_LIBS}")
endif()

if((DEFINED CMAKE_TOOLCHAIN_FILE) AND (CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg\\.cmake$"))
	set(zoo_BUILT_WITH_VCPKG_DEPS ON)
else()
	set(zoo_BUILT_WITH_VCPKG_DEPS OFF)
endif()

if(NOT DEFINED zoo_INSTALL_CMAKEDIR)
	set(zoo_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/zoo" CACHE STRING "Path to installed zoo CMake files")
endif()
