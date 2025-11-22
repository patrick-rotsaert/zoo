#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_zoo_library(squid_mysql
	SOURCES
		error.cpp
		statement.cpp
		backendconnection.cpp
		backendconnectionfactory.cpp
		connection.cpp
		detail/query.cpp
		detail/query.h
		detail/queryparameters.cpp
		detail/queryparameters.h
		detail/queryresults.cpp
		detail/queryresults.h
		detail/conversions.cpp
		detail/conversions.h
		apilinktest.cpp
	PUBLIC_HEADERS
		error.h
		statement.h
		backendconnection.h
		backendconnectionfwd.h
		backendconnectionfactory.h
		connection.h
		apilinktest.h
		detail/mysqlfwd.h
		detail/queryfwd.h
	UNIT_TEST_SOURCES
		test/unit/test_conversions.cpp
		test/unit/test_query.cpp
		test/unit/test_queryparameters.cpp
	PUBLIC_LIBRARIES
		MySQL::MySQL
		zoo::squid_core
	FIND_PACKAGE_COMPONENT
		squid
)
