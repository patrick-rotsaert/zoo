#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/options.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/vars.cmake)

if(ZOO_SQUID_WITH_MYSQL)
	# If this project is included as a subdirectory, the MySQL::MySQL target may already be defined.
	if(NOT TARGET MySQL::MySQL)
		project_find_package(unofficial-libmysql CONFIG QUIET)
		if(unofficial-libmysql_FOUND)
			add_library(MySQL::MySQL ALIAS unofficial::libmysql::libmysql)
		else()
			list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/modules)
			project_find_package(MySQL MODULE REQUIRED)
			install(
				FILES ${CMAKE_CURRENT_LIST_DIR}/modules/FindMySQL.cmake
				DESTINATION ${zoo_INSTALL_CMAKEDIR}/../modules
				COMPONENT ${COMPONENT_DEVELOPMENT}
			)
		endif()
	endif()
endif()
