#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

set(TARGET spider_example_rest)
add_zoo_executable(${TARGET} example_rest.cpp test_png_image.cpp)

target_compile_features(${TARGET} PRIVATE cxx_std_23)
target_link_libraries(${TARGET} PRIVATE zoo::spider)


##//@@
add_zoo_executable(spider_playground playground.cpp)
target_compile_features(spider_playground PRIVATE cxx_std_23)
target_link_libraries(spider_playground PRIVATE zoo::spider)

