#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

if(ZOO_SQUID_WITH_POSTGRESQL)
	add_subdirectory(demo_postgresql)
endif()

if(ZOO_SQUID_WITH_MYSQL)
	add_subdirectory(demo_mysql)
endif()

if(ZOO_SQUID_WITH_SQLITE3)
	add_subdirectory(demo_sqlite3)
endif()

add_subdirectory(demo_common)
