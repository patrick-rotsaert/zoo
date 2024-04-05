#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_subdirectory(core)

set(ZOO_SQUID_HAVE_BACKEND OFF)

if(ZOO_SQUID_WITH_POSTGRESQL)
	add_subdirectory(postgresql)
	set(ZOO_SQUID_HAVE_BACKEND ON)
endif()

if(ZOO_SQUID_WITH_MYSQL)
	add_subdirectory(mysql)
	set(ZOO_SQUID_HAVE_BACKEND ON)
endif()

if(ZOO_SQUID_WITH_SQLITE3)
	add_subdirectory(sqlite3)
	set(ZOO_SQUID_HAVE_BACKEND ON)
endif()

if(NOT ZOO_SQUID_HAVE_BACKEND)
	message(AUTHOR_WARNING "No squid bakends enabled!")
endif()

if(ZOO_BUILD_EXAMPLES)
	add_subdirectory(demo)
endif()
