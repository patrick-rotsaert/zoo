#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/policies.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/vars.cmake)

# If this project is included as a subdirectory, the ssh::ssh target may already be defined.
if(NOT TARGET ssh::ssh)
	# Prefer the system package
	find_package(ssh QUIET)
	if(ssh_FOUND)
		project_add_dependency(ssh REQUIRED)
	else()
		# sudo apt install libssh-dev
		find_library(LIBSSH_LIBRARY NAMES ssh)
		find_path(LIBSSH_INCLUDE_DIR NAMES libssh/libssh.h)
		if(LIBSSH_LIBRARY AND LIBSSH_INCLUDE_DIR)
			add_library(libssh INTERFACE)
			target_include_directories(libssh INTERFACE ${LIBSSH_INCLUDE_DIR})
			target_link_libraries(libssh INTERFACE ${LIBSSH_LIBRARY})
			add_library(ssh::ssh ALIAS libssh)
			install(TARGETS libssh EXPORT ${PROJECT_NAME}_Targets)
		elseif(ZOO_FETCH_DEPS)
			include(FetchContent)
			set(WITH_NACL OFF)
			set(WITH_EXAMPLES OFF)
			FetchContent_Declare(ssh
			  GIT_REPOSITORY https://github.com/patrick-rotsaert/libssh.git
			  GIT_BRANCH stable-0.10
			)
			FetchContent_MakeAvailable(ssh)
		else()
			message(FATAL_ERROR "Could not find libssh and fetching it was disabled by ZOO_FETCH_DEPS")
		endif()
	endif()
endif()
