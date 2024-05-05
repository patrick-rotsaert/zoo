#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include(CMakePackageConfigHelpers)

if(NOT DEFINED zoo_INSTALL_CMAKEDIR)
	set(zoo_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/zoo" CACHE STRING "Path to installed zoo CMake files")
endif()

if(BUILD_SHARED_LIBS)
	set(LINKAGE shared)
else()
	set(LINKAGE static)
endif()

if(zoo_BUILT_WITH_VCPKG_DEPS)
	install(RUNTIME_DEPENDENCY_SET zoo-runtime-deps
		COMPONENT ${COMPONENT_RUNTIME}
		NAMELINK_COMPONENT ${COMPONENT_DEVELOPMENT}
		PRE_EXCLUDE_REGEXES
			[[api-ms-win-.*]]
			[[ext-ms-.*]]
			[[^hvsifiletrust]]
			[[^pdmutilities]]
		POST_EXCLUDE_REGEXES
			[[^[A-Za-z]:[\/][Ww][Ii][Nn][Dd][Oo][Ww][Ss][\/].*]] # Windows system dlls
			[[^/lib/]] # Linux system libs
			[[^/usr/]] # Linux system libs
		)
endif()

foreach(comp ${zoo_FIND_PACKAGE_COMPONENTS})
	install(EXPORT zoo_${comp}_targets
		DESTINATION "${zoo_INSTALL_CMAKEDIR}"
		NAMESPACE zoo::
		FILE zoo-${comp}-${LINKAGE}-targets.cmake
		COMPONENT ${COMPONENT_DEVELOPMENT}
	)
endforeach()

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
