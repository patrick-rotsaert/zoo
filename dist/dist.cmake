#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

find_program(CPACK_COMMAND cpack REQUIRED)
find_program(NINJA_COMMAND ninja REQUIRED)

set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")

if(NOT BUILD_DIR)
	if(WIN32)
		set(BUILD_DIR ${SOURCE_DIR}/out/dist)
	else()
		set(BUILD_DIR /tmp/zoo-build)
	endif()
endif()

set(EXTRA_CMAKE_ARGS)

function(exec)
	list(APPEND ARGN COMMAND_ERROR_IS_FATAL ANY)
	if(DEFINED COMMAND_ECHO)
		list(APPEND ARGN COMMAND_ECHO ${COMMAND_ECHO})
	endif()
	execute_process(${ARGN})
endfunction()

function(configure BUILD_DIR BUILD_SHARED_LIBS CMAKE_BUILD_TYPE)
	message(STATUS "-----------")
	message(STATUS "Configuring shared=${BUILD_SHARED_LIBS} type=${CMAKE_BUILD_TYPE}")
	message(STATUS "-----------")
	if(WIN32)
		set(GENERATOR "Visual Studio 16 2019")
	else()
		set(GENERATOR Ninja)
	endif()
	set(ARGS
		# -Wno-dev
		-G "${GENERATOR}"
		-S ${SOURCE_DIR} -B ${BUILD_DIR}
		-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS} -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
		-DZOO_BUILD_EXAMPLES:BOOL=NO
	)
	if(WIN32)
		list(APPEND ARGS -DCMAKE_TOOLCHAIN_FILE=${SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
	endif()
	exec(COMMAND ${CMAKE_COMMAND} ${ARGS})
endfunction()

configure(${BUILD_DIR}/shared-release YES Release)
configure(${BUILD_DIR}/shared-debug   YES Debug)
configure(${BUILD_DIR}/static-release NO  Release)
configure(${BUILD_DIR}/static-debug   NO  Debug)

function(build BUILD_DIR)
	message(STATUS "--------")
	message(STATUS "Building ${BUILD_DIR}")
	message(STATUS "--------")
	exec(COMMAND ${CMAKE_COMMAND} --build ${BUILD_DIR})
endfunction()

build(${BUILD_DIR}/shared-release)
build(${BUILD_DIR}/shared-debug)
build(${BUILD_DIR}/static-release)
build(${BUILD_DIR}/static-debug)

configure_file(
	${CMAKE_CURRENT_LIST_DIR}/package.cmake.in
	${BUILD_DIR}/package.cmake
	@ONLY
)

set(ENV{LD_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/lib)
exec(
	COMMAND ${CPACK_COMMAND} --config package.cmake
	WORKING_DIRECTORY ${BUILD_DIR}
)
