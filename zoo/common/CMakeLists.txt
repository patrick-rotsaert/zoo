#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.19)

set(EXTRA_PUBLIC_LIBRARIES)

if(ZOO_THREAD_SAFE)
	list(APPEND EXTRA_PUBLIC_LIBRARIES Threads::Threads)
endif()

add_project_library(common
	TARGET_OUTPUT_NAME
		zoo_common
	SOURCES
		conversion/conversion.cpp
		lockfile/lockfile.cpp
		lockfile/lockfile_impl_posix.hpp
		logging/ibackend.cpp
		logging/spdlog_backend.cpp
		logging/logging.cpp
		misc/demangle.cpp
		misc/uuid.cpp
		misc/quoted_c.cpp
		misc/lock_types.hpp
		version.cpp
		version.h.in
		config.h.in
	PUBLIC_HEADERS
		api.h
		conversion/conversion.h
		lockfile/lockfile.h
		logging/ibackend.h
		logging/spdlog_backend.h
		logging/logging.h
		logging/log_level.h
		misc/formatters.hpp
		misc/synchronized_queue.hpp
		misc/always_false.hpp
		misc/demangle.h
		misc/demangled_type_name.hpp
		misc/is_optional.hpp
		misc/is_scoped_enum.hpp
		misc/throw_exception.h
		misc/uuid.h
		misc/quoted_c.h
	UNIT_TEST_SOURCES
		test/unit/conversion/test_conversion.cpp
		test/unit/lockfile/test_lockfile.cpp
	PUBLIC_LIBRARIES
		fmt::fmt
		spdlog::spdlog
		${EXTRA_PUBLIC_LIBRARIES}
	PUBLIC_INCLUDE_DIRS
		${PROJECT_BINARY_DIR} # for configured headers, see below
	)

#
# Build time logging levels

# Values are as in log_level.h
set(LOGGING_LEVELS trace debug info warn err critical off)

if(ZOO_DEBUG_BUILD)
	set(DEFAULT_LOGGING_LEVEL trace)
else()
	set(DEFAULT_LOGGING_LEVEL warn)
endif()

# Logging verbosity of the library
set(ZOO_LOGGING_LEVEL ${DEFAULT_LOGGING_LEVEL} CACHE STRING "Logging verbosity of the ${PROJECT_NAME} library")
set_property(CACHE ZOO_LOGGING_LEVEL PROPERTY STRINGS ${LOGGING_LEVELS})

configure_file(
	version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/version.h
)

configure_file(
	config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

if(ZOO_INSTALL)
	install_project_header(${CMAKE_CURRENT_BINARY_DIR}/version.h ${PROJECT_BINARY_DIR})
	install_project_header(${CMAKE_CURRENT_BINARY_DIR}/config.h ${PROJECT_BINARY_DIR})
endif()

if(ZOO_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
