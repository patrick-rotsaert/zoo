#
# Copyright (C) 2022-2024 Patrick Rotsaert
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#

add_zoo_library(squid_core
	SOURCES
		parameter.cpp
		result.cpp
		error.cpp
		ibackendconnection.cpp
		ibackendconnectionfactory.cpp
		ibackendstatement.cpp
		connection.cpp
		connectionpool.cpp
		basicstatement.cpp
		statement.cpp
		preparedstatement.cpp
		transaction.cpp
		apilinktest.cpp
	PUBLIC_HEADERS
		parameter.h
		result.h
		error.h
		ibackendstatement.h
		ibackendstatementfwd.h
		ibackendconnection.h
		ibackendconnectionfwd.h
		ibackendconnectionfactory.h
		ibackendconnectionfactoryfwd.h
		connection.h
		connectionfwd.h
		connectionpool.h
		connectionpoolfwd.h
		basicstatement.h
		statement.h
		preparedstatement.h
		preparedstatementfwd.h
		transaction.h
		types.h
		detail/parameterbinder.h
		detail/resultbinder.h
		detail/type_traits.h
		detail/bind_oarchive.h
		detail/bind_iarchive.h
		apilinktest.h
	UNIT_TEST_SOURCES
		test/unit/test_parameter.cpp
		test/unit/test_result.cpp
	PUBLIC_LIBRARIES
		zoo::zoocommon
	FIND_PACKAGE_COMPONENT
		squid
)
