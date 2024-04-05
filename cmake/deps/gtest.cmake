#
# Copyright (C) 2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

include_guard(GLOBAL)

include(${PROJECT_SOURCE_DIR}/cmake/policies.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/vars.cmake)

if(ZOO_TEST)
	include(FetchContent)
	set(INSTALL_GTEST OFF)
	FetchContent_Declare(
		googletest
		URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
	)
	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)
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
