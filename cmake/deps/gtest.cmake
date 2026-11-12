#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/options.cmake)

# If this project is included as a subdirectory, the GTest targets may already be defined.
if(ZOO_TEST AND NOT TARGET GTest::gtest)
	# libgtest-dev
	find_package(GTest CONFIG REQUIRED)
endif()

# A GTest bundle with gmock compiled into gtest may provide no GTest::gmock_main.
if(ZOO_TEST AND NOT TARGET GTest::gmock_main)
	add_library(GTest::gmock_main INTERFACE IMPORTED)
	set_target_properties(GTest::gmock_main PROPERTIES INTERFACE_LINK_LIBRARIES GTest::gtest_main)
endif()

if(ZOO_TEST AND ZOO_RUN_UNIT_TESTS_ON_BUILD)
	function(run_unit_test_on_build TARGET)
		add_test(NAME ${TARGET} COMMAND ${TARGET})
		add_custom_command(TARGET ${TARGET} POST_BUILD
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> -R "${TARGET}" --output-on-failure
		)
	endfunction()
else()
	function(run_unit_test_on_build TARGET)
	endfunction()
endif()

include(GoogleTest)
