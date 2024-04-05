#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

if((NOT DEFINED ${PROJECT_NAME}_VERSION_MAJOR) OR
   (NOT DEFINED ${PROJECT_NAME}_VERSION_MINOR) OR
   (NOT DEFINED ${PROJECT_NAME}_VERSION_PATCH))
	message(FATAL_ERROR "Version not defined for project '${PROJECT_NAME}'")
endif()

math(EXPR ${PROJECT_NAME}_VERSION_NUMBER "(${${PROJECT_NAME}_VERSION_PATCH} + ${${PROJECT_NAME}_VERSION_MINOR} * 10000 + ${${PROJECT_NAME}_VERSION_MAJOR} * 10000 * 100)")
