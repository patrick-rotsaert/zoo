#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_zoo_library(bitcask
	SOURCES
		bitcask.cpp
		datadir.cpp
		datadir.h
		datafile.cpp
		datafile.h
		hintfile.cpp
		hintfile.h
		keydir.cpp
		keydir.h
		basictypes.h
		crc32.cpp
		crc32.h
		file.cpp
		file.h
		hton.h
		apilinktest.cpp
	PUBLIC_HEADERS
		bitcask.h
		apilinktest.h
	UNIT_TEST_SOURCES
		test/unit/test_bitcask.cpp
	PUBLIC_LIBRARIES
		zoo::zoocommon
		fmt::fmt
	FIND_PACKAGE_COMPONENT
		bitcask
)

if(ZOO_TEST)
	target_link_libraries(bitcask_unit_test PRIVATE Boost::filesystem)
endif()

if(ZOO_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
