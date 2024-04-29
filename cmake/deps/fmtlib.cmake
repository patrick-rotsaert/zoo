#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

# If this project is included as a subdirectory, the fmt::fmt target may already be defined.
if(NOT TARGET fmt::fmt)
	project_find_package(fmt CONFIG REQUIRED)
endif()
