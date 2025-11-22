#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

if(NOT UNIX)
	return()
endif()

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses)
if(NOT Curses_FOUND)
	return()
endif()

include(CheckIncludeFile)
check_include_file("term.h" HAVE_TERM_H)
check_include_file("termio.h" HAVE_TERMIO_H)
check_include_file("termios.h" HAVE_TERMIOS_H)

configure_file(tuiconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/tuiconfig.h @ONLY)

add_zoo_library(tui
	SOURCES
		apilinktest.cpp
		asioterminal.cpp
		asioinput.cpp
		asioinput.h
		asiotimer.cpp
		asiotimer.h
		tuisize.cpp
		tuiposition.cpp
	PUBLIC_HEADERS
		apilinktest.h
		asioterminal.h
		character.h
		tuiattribute.h
		tuisize.h
		tuiposition.h
		keycode.h
		keymodifier.h
	PUBLIC_LIBRARIES
		zoo::zoocommon
		fmt::fmt
	PRIVATE_INCLUDE_DIRS
		${CMAKE_CURRENT_BINARY_DIR}
	FIND_PACKAGE_COMPONENT
		tui
)

# if(ZOO_BUILD_EXAMPLES)
# 	add_subdirectory(examples)
# endif()
