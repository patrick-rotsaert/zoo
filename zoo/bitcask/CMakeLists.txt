#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_project_library(bitcask
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
	# TODO: unit tests
	PUBLIC_HEADERS
		bitcask.h
		apilinktest.h
	PUBLIC_LIBRARIES
		zoo::common
		fmt::fmt
	FIND_PACKAGE_COMPONENT
		bitcask
)

if(ZOO_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
