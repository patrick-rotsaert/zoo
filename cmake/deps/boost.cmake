#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/vars.cmake)

set(_boost_libs date_time system filesystem thread url json regex)

set(_found_all_boost_targets ON)
foreach(lib ${_boost_libs})
	if(NOT TARGET Boost::${lib})
		set(_found_all_boost_targets OFF)
		break()
	endif()
endforeach()

# If this project is included as a subdirectory, the Boost targets may already be defined.
if(NOT _found_all_boost_targets)
	project_find_package(Boost REQUIRED COMPONENTS ${_boost_libs})
endif()
