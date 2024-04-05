#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.19)

add_project_library(bitcask
	TARGET_OUTPUT_NAME
		${PROJECT_NAME}_bitcask
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
		${EXTRA_SOURCES}
	PUBLIC_HEADERS
		bitcask.h
	PUBLIC_LIBRARIES
		zoo::common
		fmt::fmt
)
