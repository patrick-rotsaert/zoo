#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/options.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/vars.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/version.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/components.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps.cmake)

# On windows, set all output directories to ${CMAKE_BINARY_DIR}
# Without this, we would need to do the following for unit test targets using shared build type.
# ```
# 	add_custom_command(
# 		TARGET ${TARGET} POST_BUILD
# 		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>
# 		COMMAND_EXPAND_LISTS
# 	)
# ```
# Otherwise the unit tests cannot be run because they would not find the used DLL's.
# However, having the dependency DLL's copied to multiple directories makes install(RUNTIME_DEPENDENCY_SET ...)
# fail due to conflicting dependencies.
if(WIN32)
	foreach(d RUNTIME LIBRARY ARCHIVE PDB)
		set(CMAKE_${d}_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
		foreach(t DEBUG RELEASE)
			set(CMAKE_${d}_OUTPUT_DIRECTORY_${t} ${CMAKE_${d}_OUTPUT_DIRECTORY})
		endforeach()
	endforeach()
endif()

function(install_zoo_header HEADER BASE_DIR)
	file(REAL_PATH ${HEADER} HEADER_REALPATH)
	file(RELATIVE_PATH HEADER_RELPATH ${BASE_DIR} ${HEADER_REALPATH})
	get_filename_component(DIR ${HEADER_RELPATH} DIRECTORY)
	install(
		FILES ${HEADER}
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${DIR}
		COMPONENT ${COMPONENT_DEVELOPMENT}
	)
endfunction()

function(add_zoo_executable TARGET)
	add_executable(${TARGET} ${ARGN})

	if (WIN32)
		target_compile_definitions(${TARGET} PRIVATE "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:$<BUILD_INTERFACE:_WIN32_WINNT=0x0601>>")
	endif()
endfunction()

set(zoo_FIND_PACKAGE_COMPONENTS "" CACHE STRING "" FORCE)
mark_as_advanced(zoo_FIND_PACKAGE_COMPONENTS)

function(add_zoo_library TARGET)
	set(OPTIONS SKIP_INSTALL)
	set(ONE_VALUE_ARGS FIND_PACKAGE_COMPONENT)
	set(MULTI_VALUE_ARGS
		SOURCES
		PUBLIC_HEADERS
		UNIT_TEST_SOURCES
		MOCK_SOURCES
		PRIVATE_DEFINITIONS
		PRIVATE_INCLUDE_DIRS
		PUBLIC_INCLUDE_DIRS
		PRIVATE_LIBRARIES
		PUBLIC_LIBRARIES
		MSVC_PRIVATE_COMPILER_OPTIONS
		MSVC_PUBLIC_COMPILER_OPTIONS
	)
	cmake_parse_arguments(P "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

	string(TOUPPER ${TARGET} TARGET_UC)
	configure_file(${PROJECT_SOURCE_DIR}/zoo/common/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

	if(ZOO_INSTALL)
		install_zoo_header(${CMAKE_CURRENT_BINARY_DIR}/config.h ${PROJECT_BINARY_DIR})
	endif()

	# Set the target output name
	set(TARGET_OUTPUT_NAME zoo_${TARGET})
	set(DEBUG_POSTFIX)
	set(RELEASE_POSTFIX)

	if(MSVC)
		set(TOOLSET_NAME vc${MSVC_TOOLSET_VERSION})
		if(BUILD_SHARED_LIBS)
			set(DEBUG_POSTFIX -${TOOLSET_NAME}-gd)
			set(RELEASE_POSTFIX -${TOOLSET_NAME})
		else()
			set(DEBUG_POSTFIX -${TOOLSET_NAME}-gd-s)
			set(RELEASE_POSTFIX -${TOOLSET_NAME}-s)
		endif()
		set(DEBUG_POSTFIX ${DEBUG_POSTFIX}-${zoo_ARCH}-${${PROJECT_NAME}_VERSION_MAJOR}_${${PROJECT_NAME}_VERSION_MINOR})
		set(RELEASE_POSTFIX ${RELEASE_POSTFIX}-${zoo_ARCH}-${${PROJECT_NAME}_VERSION_MAJOR}_${${PROJECT_NAME}_VERSION_MINOR})
	endif()

	# Create a list of compile and link targets
	set(COMPILE_TARGETS)
	set(LINK_TARGETS ${TARGET})

	# Add the library target
	if(P_UNIT_TEST_SOURCES AND ZOO_TEST)
		add_library(${TARGET}_objects OBJECT ${P_SOURCES} ${P_PUBLIC_HEADERS})
		list(APPEND COMPILE_TARGETS ${TARGET}_objects)

		add_library(${TARGET} $<TARGET_OBJECTS:${TARGET}_objects>)
		list(APPEND COMPILE_TARGETS ${TARGET})

		add_zoo_executable(${TARGET}_unit_test ${P_UNIT_TEST_SOURCES} $<TARGET_OBJECTS:${TARGET}_objects>)

		if (P_MOCK_SOURCES)
			target_sources(${TARGET}_unit_test PRIVATE ${P_MOCK_SOURCES})
			target_link_libraries(${TARGET}_unit_test PRIVATE GTest::gmock GTest::gmock_main)
		endif()

		list(APPEND COMPILE_TARGETS ${TARGET}_unit_test)

		target_link_libraries(${TARGET}_unit_test PRIVATE GTest::gtest GTest::gtest_main)

		list(APPEND LINK_TARGETS ${TARGET}_unit_test)

		gtest_discover_tests(${TARGET}_unit_test DISCOVERY_TIMEOUT 30)

		run_unit_test_on_build(${TARGET}_unit_test)
	else()
		add_library(${TARGET} ${P_SOURCES} ${P_PUBLIC_HEADERS})
		list(APPEND COMPILE_TARGETS ${TARGET})
	endif()

	# Add library alias
	add_library(zoo::${TARGET} ALIAS ${TARGET})

	# Hide all symbols by default
	set_target_properties(${TARGET} PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		C_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)

	foreach(TARGET ${COMPILE_TARGETS})
		# Specify the C++ standard
		target_compile_features(${TARGET} PRIVATE cxx_std_20)

		# PIC
		set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE True)

		# Set the compiler warning options
		set(GCC_WARN_OPTS -Wall -Wextra -pedantic)
		set(MSVC_WARN_OPTS /W4)
		if(ZOO_TREAT_WARNINGS_AS_ERRORS)
			list(APPEND GCC_WARN_OPTS -Werror)
			list(APPEND MSVC_WARN_OPTS /WX)
		endif()
		target_compile_options(${TARGET} PRIVATE
			"$<$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>:$<BUILD_INTERFACE:${GCC_WARN_OPTS}>>"
			"$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:$<BUILD_INTERFACE:${MSVC_WARN_OPTS}>>"
		)

		# GDB
		target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Debug>:$<$<CXX_COMPILER_ID:GNU>:-O0;-ggdb3>>)
		target_compile_options(${TARGET} PRIVATE $<$<CONFIG:RelWithDebInfo>:$<$<CXX_COMPILER_ID:GNU>:-O2;-ggdb3>>)

		# Windows
		target_compile_definitions(${TARGET} PRIVATE "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:$<BUILD_INTERFACE:_WIN32_WINNT=0x0601>>")

		# Set compiler definitions
		if(P_PRIVATE_DEFINITIONS)
			target_compile_definitions(${TARGET} PRIVATE ${P_PRIVATE_DEFINITIONS})
		endif()

		if(BUILD_SHARED_LIBS)
			target_compile_definitions(${TARGET}
				PUBLIC ZOO_SHARED
				PRIVATE ZOO_${TARGET_UC}_EXPORTS
			)
		endif()

		# Set include dirs
		if(P_PRIVATE_INCLUDE_DIRS)
			foreach(DIR ${P_PRIVATE_INCLUDE_DIRS})
				target_include_directories(${TARGET} PRIVATE $<BUILD_INTERFACE:${DIR}>)
			endforeach()
		endif()
		if(P_PUBLIC_INCLUDE_DIRS)
			foreach(DIR ${P_PUBLIC_INCLUDE_DIRS})
				target_include_directories(${TARGET} PUBLIC $<BUILD_INTERFACE:${DIR}>)
			endforeach()
		endif()

		# Set the common include directories
		target_include_directories(${TARGET} PUBLIC
			$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
			$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}> # for generated headers
			$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
		)

		# Link libraries
		if(P_PUBLIC_LIBRARIES)
			target_link_libraries(${TARGET} PUBLIC ${P_PUBLIC_LIBRARIES})
		endif()
		if(P_PRIVATE_LIBRARIES)
			target_link_libraries(${TARGET} PRIVATE ${P_PRIVATE_LIBRARIES})
		endif()

		# MSVC compiler options
		if(P_MSVC_PRIVATE_COMPILER_OPTIONS)
			target_compile_options(${TARGET} PRIVATE
				"$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:$<BUILD_INTERFACE:${P_MSVC_PRIVATE_COMPILER_OPTIONS}>>"
			)
		endif()
		if(P_MSVC_PUBLIC_COMPILER_OPTIONS)
			target_compile_options(${TARGET} PUBLIC
				"$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:$<BUILD_INTERFACE:${P_MSVC_PUBLIC_COMPILER_OPTIONS}>>"
			)
		endif()
	endforeach()

	foreach(TARGET ${LINK_TARGETS})
		set_target_properties(${TARGET} PROPERTIES
			DEBUG_POSTFIX "${DEBUG_POSTFIX}"
			RELEASE_POSTFIX "${RELEASE_POSTFIX}"
		)
	endforeach()

	# Set other target properties
	set_target_properties(${TARGET} PROPERTIES
		VERSION ${${PROJECT_NAME}_VERSION}
		SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR}
		OUTPUT_NAME "${TARGET_OUTPUT_NAME}"
	)

	if(MSVC)
		if(BUILD_SHARED_LIBS)
			set_target_properties(${TARGET} PROPERTIES
				PDB_NAME_DEBUG ${TARGET_OUTPUT_NAME}${DEBUG_POSTFIX}
				PDB_NAME_RELEASE ${TARGET_OUTPUT_NAME}${RELEASE_POSTFIX}
				PDB_OUTPUT_DIRECTORY CMAKE_RUNTIME_OUTPUT_DIRECTORY
			)
		else()
			set_target_properties(${TARGET} PROPERTIES
				COMPILE_PDB_NAME_DEBUG ${TARGET_OUTPUT_NAME}${DEBUG_POSTFIX}
				COMPILE_PDB_NAME_RELEASE ${TARGET_OUTPUT_NAME}${RELEASE_POSTFIX}
				COMPILE_PDB_OUTPUT_DIRECTORY CMAKE_LIBRARY_OUTPUT_DIRECTORY
			)
		endif()
	endif()

	if(ZOO_INSTALL AND (NOT P_SKIP_INSTALL))
		if(NOT P_FIND_PACKAGE_COMPONENT)
			message(FATAL_ERROR "A FIND_PACKAGE_COMPONENT argument is required for installation")
		endif()

		set(tmp ${zoo_FIND_PACKAGE_COMPONENTS})
		list(APPEND tmp ${P_FIND_PACKAGE_COMPONENT})
		list(REMOVE_DUPLICATES tmp)
		set(zoo_FIND_PACKAGE_COMPONENTS ${tmp} CACHE STRING "" FORCE)

		# Install the library
		set(zoo_RUNTIME_DEPENDENCY_SET_ARGS)
		if(zoo_BUILT_WITH_VCPKG_DEPS)
			list(APPEND zoo_RUNTIME_DEPENDENCY_SET_ARGS RUNTIME_DEPENDENCY_SET zoo-runtime-deps)
		endif()

		install(TARGETS ${TARGET}
			EXPORT zoo_${P_FIND_PACKAGE_COMPONENT}_targets
			${zoo_RUNTIME_DEPENDENCY_SET_ARGS}
			RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
			        COMPONENT ${COMPONENT_RUNTIME}
			LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
			        COMPONENT ${COMPONENT_RUNTIME}
			        NAMELINK_COMPONENT ${COMPONENT_DEVELOPMENT}
			ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
			        COMPONENT ${COMPONENT_DEVELOPMENT}
		)

		# Install public header files
		foreach(HEADER ${P_PUBLIC_HEADERS})
			install_zoo_header(${HEADER} ${PROJECT_SOURCE_DIR})
		endforeach()

		# Install the PDB files
		if(MSVC)
			if(BUILD_SHARED_LIBS)
				install(FILES $<TARGET_PDB_FILE:${TARGET}>
					COMPONENT ${COMPONENT_DEVELOPMENT}
					DESTINATION ${CMAKE_INSTALL_BINDIR}
					OPTIONAL
				)
			else()
				get_target_property(TARGET_COMPILE_PDB_OUTPUT_DIRECTORY ${TARGET} COMPILE_PDB_OUTPUT_DIRECTORY)
				if(TARGET_COMPILE_PDB_OUTPUT_DIRECTORY)
					get_target_property(TARGET_COMPILE_PDB_NAME_DEBUG ${TARGET} COMPILE_PDB_NAME_DEBUG)
					if (TARGET_COMPILE_PDB_NAME_DEBUG)
						install(FILES ${TARGET_COMPILE_PDB_OUTPUT_DIRECTORY}/${TARGET_COMPILE_PDB_NAME_DEBUG}.pdb
							COMPONENT ${COMPONENT_DEVELOPMENT}
							DESTINATION ${CMAKE_INSTALL_LIBDIR}
							OPTIONAL
						)
					endif()
					get_target_property(TARGET_COMPILE_PDB_NAME_RELEASE ${TARGET} COMPILE_PDB_NAME_RELEASE)
					if (TARGET_COMPILE_PDB_NAME_RELEASE)
						install(FILES ${TARGET_COMPILE_PDB_OUTPUT_DIRECTORY}/${TARGET_COMPILE_PDB_NAME_RELEASE}.pdb
							COMPONENT ${COMPONENT_DEVELOPMENT}
							DESTINATION ${CMAKE_INSTALL_LIBDIR}
							OPTIONAL
						)
					endif()
				endif()
			endif()
		endif()
	endif()
endfunction()
