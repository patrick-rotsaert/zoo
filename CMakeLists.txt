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

include(cmake/project.cmake)

add_subdirectory(zoo)

if(ZOO_INSTALL)
	add_subdirectory(install)
	if(ZOO_CPACK)
		add_subdirectory(cpack)
	endif()
endif()
