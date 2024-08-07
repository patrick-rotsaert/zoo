#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_zoo_library(squid_postgresql
	SOURCES
		error.cpp
		statement.cpp
		backendconnection.cpp
		backendconnectionfactory.cpp
		connection.cpp
		detail/conversions.cpp
		detail/conversions.h
		detail/connectionchecker.cpp
		detail/connectionchecker.h
		detail/query.cpp
		detail/query.h
		detail/queryparameters.cpp
		detail/queryparameters.h
		detail/queryresults.cpp
		detail/queryresults.h
		detail/execresult.h
		detail/ipqapi.cpp
		detail/ipqapi.h
		detail/pqapi.cpp
		detail/pqapi.h
		apilinktest.cpp
	PUBLIC_HEADERS
		error.h
		statement.h
		backendconnection.h
		backendconnectionfactory.h
		connection.h
		detail/libpqfwd.h
		apilinktest.h
	MOCK_SOURCES
		detail/pqapimock.cpp
		detail/pqapimock.h
	UNIT_TEST_SOURCES
		test/unit/test_error.cpp
		test/unit/test_backendconnection.cpp
		test/unit/test_backendconnectionfactory.cpp
		test/unit/test_connection.cpp
		detail/test/unit/test_connectionchecker.cpp
		detail/test/unit/test_conversions.cpp
		detail/test/unit/test_query.cpp
		detail/test/unit/test_queryparameters.cpp
		detail/test/unit/test_queryresults.cpp
	PUBLIC_LIBRARIES
		PostgreSQL::PostgreSQL
		zoo::squid_core
	FIND_PACKAGE_COMPONENT
		squid
)
