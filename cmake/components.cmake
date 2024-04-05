#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

# Set the component names
# These are prefixed with ${PROJECT_NAME} to avoid name clashes with component names
# of projects that include this library as a subdirectory.
set(COMPONENT_RUNTIME ${PROJECT_NAME}_runtime)
set(COMPONENT_DEVELOPMENT ${PROJECT_NAME}_development)

string(TOUPPER ${COMPONENT_RUNTIME} COMPONENT_RUNTIME_UC)
string(TOUPPER ${COMPONENT_DEVELOPMENT} COMPONENT_DEVELOPMENT_UC)
