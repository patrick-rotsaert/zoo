#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.19)

include(CMakePackageConfigHelpers)

if(NOT DEFINED zoo_INSTALL_CMAKEDIR)
	set(zoo_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/zoo" CACHE STRING "Path to zoo CMake files")
endif()

if(BUILD_SHARED_LIBS)
	set(LINKAGE shared)
else()
	set(LINKAGE static)
endif()

install(EXPORT zoo_targets
	DESTINATION "${zoo_INSTALL_CMAKEDIR}"
	NAMESPACE zoo::
	FILE zoo-${LINKAGE}-targets.cmake
	COMPONENT ${COMPONENT_DEVELOPMENT}
)

write_basic_package_version_file(
    zoo-config-version.cmake
    COMPATIBILITY SameMajorVersion
)

# PROJECT_DEPENDENCIES is a placeholder in project-config.cmake.in
set(PROJECT_DEPENDENCIES ${zoo_DEPENDENCIES})

configure_file(
	${PROJECT_SOURCE_DIR}/cmake/project-config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/zoo-config.cmake
	@ONLY
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/zoo-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/zoo-config-version.cmake
	DESTINATION "${zoo_INSTALL_CMAKEDIR}"
	COMPONENT ${COMPONENT_DEVELOPMENT}
)