#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

set(TARGET demo_mysql)
add_executable(${TARGET} demo_mysql.cpp)

target_compile_features(${TARGET} PRIVATE cxx_std_20)
target_link_libraries(${TARGET} PRIVATE zoo::squid::mysql)
target_link_libraries(${TARGET} PRIVATE zoo::squid::demo::common)
