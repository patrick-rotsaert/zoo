#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.19)

add_project_library(squid_demo_common
	SOURCES
		demo_common.cpp
		demo_common.h
	PUBLIC_LIBRARIES
		zoo::squid_core
	SKIP_INSTALL
)
