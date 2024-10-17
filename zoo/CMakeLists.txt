#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_subdirectory(common)

if(ZOO_WITH_BITCASK)
	add_subdirectory(bitcask)
endif()

if(ZOO_WITH_FS)
	add_subdirectory(fs)
endif()

if(ZOO_WITH_SPIDER)
	add_subdirectory(spider)
endif()

if(ZOO_WITH_SQUID)
	add_subdirectory(squid)
endif()

if(ZOO_WITH_TUI)
	add_subdirectory(tui)
endif()
