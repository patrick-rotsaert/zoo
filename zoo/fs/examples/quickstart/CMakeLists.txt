#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

set(TARGET fs_quickstart)
add_zoo_executable(${TARGET} quickstart.cpp)

target_compile_features(${TARGET} PRIVATE cxx_std_20)
target_link_libraries(${TARGET} PRIVATE zoo::fs_local)
target_link_libraries(${TARGET} PRIVATE zoo::fs_sftp)
