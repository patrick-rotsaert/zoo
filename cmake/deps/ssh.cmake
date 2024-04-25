#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/options.cmake)

if(ZOO_WITH_FS)
	# If this project is included as a subdirectory, the ssh::ssh target may already be defined.
	if(NOT TARGET ssh::ssh)
		project_find_package(libssh CONFIG REQUIRED)
		add_library(ssh::ssh ALIAS ssh)
	endif()
endif()
