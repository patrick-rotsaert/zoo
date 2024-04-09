#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_project_library(fs_local
	SOURCES
		local_access.cpp
		local_watcher.cpp
		local_watcher.h
		local_file.cpp
		local_file.h
		make_attributes.cpp
		make_attributes.h
		make_direntry.cpp
		make_direntry.h
	PUBLIC_HEADERS
		local_access.h
	UNIT_TEST_SOURCES
		test/unit/test_local_access.cpp
		test/unit/test_make_attributes.cpp
		test/unit/test_make_direntry.cpp
		test/unit/local_fs_test_fixture.cpp
		test/unit/local_fs_test_fixture.h
		# TODO? test/unit/test_local_watcher.cpp
	PRIVATE_DEFINITIONS
		BOOST_STACKTRACE_LINK
	PRIVATE_INCLUDE_DIRS
		${CMAKE_CURRENT_BINARY_DIR}/../..
	PUBLIC_LIBRARIES
		zoo::fs_core
	PRIVATE_LIBRARIES
		fmt::fmt
		Boost::thread
	FIND_PACKAGE_COMPONENT
		fs
)
