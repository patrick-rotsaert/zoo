#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

set(zoo_DEPENDENCIES "" CACHE STRING "" FORCE)
mark_as_advanced(zoo_DEPENDENCIES)

function(project_add_dependency)
	list(JOIN ARGN "," csv)
	set(tmp ${zoo_DEPENDENCIES})
	list(APPEND tmp ${csv})
	set(zoo_DEPENDENCIES ${tmp} CACHE STRING "" FORCE)
endfunction()

function(project_find_package NAME)
	set(args ${NAME} ${ARGN})
	find_package(${args})
	if(${NAME}_FOUND)
		project_add_dependency(${args})
	endif()
	set(${NAME}_FOUND ${${NAME}_FOUND} PARENT_SCOPE)
endfunction()

include(${CMAKE_CURRENT_LIST_DIR}/deps/threads.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/gtest.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/fmtlib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/spdlog.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/ssh.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/boost.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/postgresql.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/mysql.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/sqlite3.cmake)
