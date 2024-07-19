#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

set(EXTRA_PUBLIC_LIBRARIES)

if(ZOO_THREAD_SAFE)
	list(APPEND EXTRA_PUBLIC_LIBRARIES Threads::Threads)
endif()

if(WIN32)
	# for boost::uuid
	list(APPEND EXTRA_PUBLIC_LIBRARIES bcrypt)
endif()

add_zoo_library(common
	SOURCES
		conversion/conversion.cpp
		lockfile/lockfile.cpp
		lockfile/lockfile_impl_posix.hpp
		lockfile/lockfile_impl_windows.hpp
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
		apilinktest.cpp
	PUBLIC_HEADERS
		api.h
		compat.h
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
		apilinktest.h
	UNIT_TEST_SOURCES
		test/unit/conversion/test_conversion.cpp
		test/unit/lockfile/test_lockfile.cpp
	PUBLIC_LIBRARIES
		fmt::fmt
		spdlog::spdlog
		Boost::boost
		${EXTRA_PUBLIC_LIBRARIES}
	PUBLIC_INCLUDE_DIRS
		${PROJECT_BINARY_DIR} # for configured headers, see below
	FIND_PACKAGE_COMPONENT
		common
)

configure_file(
	version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/version.h
)

if(ZOO_INSTALL)
	install_zoo_header(${CMAKE_CURRENT_BINARY_DIR}/version.h ${PROJECT_BINARY_DIR})
endif()

if(ZOO_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
