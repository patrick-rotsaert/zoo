#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/policies.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/vars.cmake)

set(_boost_libs date_time system filesystem thread url json)
set(_boost_version 1.81)
set(_boost_git_tag boost-1.81.0)

set(_found_all_boost_targets ON)
foreach(lib ${_boost_libs})
	if(NOT TARGET Boost::${lib})
		set(_found_all_boost_targets OFF)
		break()
	endif()
endforeach()

# If this project is included as a subdirectory, the Boost targets may already be defined.
if(NOT _found_all_boost_targets)
	find_package(Boost ${_boost_version} QUIET COMPONENTS ${_boost_libs})
	set(_found_all_boost_targets ON)
	foreach(lib ${_boost_libs})
		string(TOUPPER ${lib} lib_uc)
		if(NOT Boost_${lib_uc}_FOUND)
			set(_found_all_boost_targets OFF)
			break()
		endif()
	endforeach()
	if(_found_all_boost_targets)
		project_add_dependency(Boost ${_boost_version} REQUIRED COMPONENTS ${_boost_libs})
	elseif(ZOO_FETCH_DEPS)
		include(FetchContent)
		set(BOOST_INCLUDE_LIBRARIES ${_boost_libs})
		set(BOOST_ENABLE_CMAKE ON)
		FetchContent_Declare(
			Boost
			GIT_REPOSITORY https://github.com/boostorg/boost.git
			GIT_TAG ${_boost_git_tag}
			GIT_SHALLOW TRUE
		)
		FetchContent_MakeAvailable(Boost)
	else()
		message(FATAL_ERROR "Could not find Boost or not all of its required components and fetching it was disabled by ZOO_FETCH_DEPS")
	endif()
endif()
