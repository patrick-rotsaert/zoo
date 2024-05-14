#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_project_library(fs_core
	SOURCES
		iaccess.cpp
		iinterruptor.cpp
		ifile.cpp
		iwatcher.cpp
		attributes.cpp
		source.cpp
		destination.cpp
		operations.cpp
		make_dest_path.cpp
		make_dest_path.h
		exceptions.cpp
		noop_interruptor.cpp
		apilinktest.cpp
	PUBLIC_HEADERS
		iaccess.h
		iinterruptor.h
		ifile.h
		iwatcher.h
		direntry.h
		attributes.h
		fspath.h
		source.h
		destination.h
		operations.h
		exceptions.h
		noop_interruptor.h
		apilinktest.h
	UNIT_TEST_SOURCES
		test/unit/test_attributes.cpp
		test/unit/test_destination.cpp
		test/unit/test_exceptions.cpp
		test/unit/test_i_interruptor.cpp
		test/unit/test_make_dest_path.cpp
		test/unit/test_operations.cpp
		test/unit/test_source.cpp
	MOCK_SOURCES
		test/unit/mock_access.cpp
		test/unit/mock_access.h
		test/unit/mock_file.cpp
		test/unit/mock_file.h
	PUBLIC_LIBRARIES
		zoo::common
		Boost::filesystem
	PRIVATE_LIBRARIES
		fmt::fmt
	FIND_PACKAGE_COMPONENT
		fs
)
