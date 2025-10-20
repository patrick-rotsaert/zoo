#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

set(TARGET demo_postgresql)
add_zoo_executable(${TARGET} demo_postgresql.cpp)

target_compile_features(${TARGET} PRIVATE cxx_std_23)
target_link_libraries(${TARGET} PRIVATE zoo::squid_postgresql)
target_link_libraries(${TARGET} PRIVATE zoo::squid_demo_common)
