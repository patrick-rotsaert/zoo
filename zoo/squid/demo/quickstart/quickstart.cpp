//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/connection.h"
#include "zoo/squid/mysql/connection.h"
#include "zoo/squid/sqlite3/connection.h"
#include "zoo/squid/core/connectionpool.h"
#include "zoo/squid/core/connection.h"
#include "zoo/squid/core/statement.h"
#include "zoo/squid/core/preparedstatement.h"
#include "zoo/squid/sqlite3/backendconnectionfactory.h"

#include <boost/exception/diagnostic_information.hpp>

#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <iomanip>

using namespace zoo::squid;

void connect_to_postgresql()
{
	constexpr auto connection_info = "host=localhost port=54321 dbname=quickstart user=postgres password=Pass123";

	postgresql::connection connection{ connection_info };
	std::cout << "opened database " << std::quoted(connection_info) << "\n";
}

void connect_to_mysql()
{
	constexpr auto connection_info = R"~(host=127.0.0.1 port=13306 db=quickstart user=myuser passwd="Pass 123")~";

	mysql::connection connection{ connection_info };
	std::cout << "opened database " << std::quoted(connection_info) << "\n";
}

void connect_to_sqlite()
{
	constexpr auto path = "quickstart.db";
	// constexpr auto path = ":memory:";

	sqlite::connection connection{ path };
	std::cout << "opened database " << std::quoted(path) << "\n";
}

void use_connection_pool()
{
	using namespace std::chrono_literals;

	// Remark: a connection pool isn't very useful in single threaded code,
	//         but this is only for demonstration purposes.

	// Create a pool of 5 SQLite connections.
	constexpr auto  path = "quickstart.db";
	connection_pool pool{ sqlite::backend_connection_factory{}, path, 5 };

	// Acquire a connection from the pool.
	// Will throw if no connections are available.
	{
		connection conn{ pool };
		// When `conn` goes out of scope, the connection is released back to the pool.
	}

	// Acquire a connection from the pool with a timeout of 250ms.
	// Will throw if no connections are available within the given timeout.
	{
		connection conn{ pool, 250ms };
	}

	// The non throwing variants (static methods) return an optional connection.
	{
		// Returns std::nullopt if no connection is immediately available.
		auto conn = connection::create(pool);
		if (conn)
		{
			// use `conn.value()`
		}
	}
	{
		// Returns std::nullopt if no connection is available within the given timeout.
		auto conn = connection::create(pool, 250ms);
		if (conn)
		{
			// use `conn.value()`
		}
	}
}

void run_non_parameterized_statements(connection& conn)
{
	conn.execute(R"~(
		CREATE TABLE person(
			first_name TEXT,
			last_name TEXT,
			date_of_birth DATE
		)
	)~");

	conn.execute(R"~(
		INSERT INTO person(first_name, last_name, date_of_birth)
		VALUES ('John', 'Doe', '2000-12-31')
	)~");
}

void run_one_off_statements(connection& conn)
{
	{
		statement st{ conn, R"~(
			INSERT INTO person(first_name, last_name, date_of_birth)
			VALUES (:fname, :lname, :dob)
		)~" };
		st.bind("fname", "Jane");
		st.bind("lname", "Doe");
		st.bind("dob", boost::gregorian::date{ 1998, 1, 31 });
		st.execute();

		// Binding parameters (by value) and statement execution can be combined.
		st.bind_execute({ { "fname", "Jokke" }, { "lname", "Doe" }, { "dob", boost::gregorian::date{ 1999, 12, 31 } } });
	}

	{
		statement st{ conn, "SELECT last_name, first_name FROM person WHERE last_name = :last" };
		st.bind("last", "Doe");

		std::string first{}, last{};
		// Binding the results is done by column name or in select order (but not both).
#if 0
		// Bind the results by name.
		st.bind_result("first_name", first);
		st.bind_result("last_name", last);
#else
		// Bind the results in the order as they appear in the query.
		st.bind_results(last, first);
#endif

		st.execute();
		while (st.fetch())
		{
			std::cout << "first=" << std::quoted(first) << ", last=" << std::quoted(last) << '\n';
		}
	}
}

void run_recurring_statement(connection& conn)
{
	{
		prepared_statement st{ conn, R"~(
			INSERT INTO person(first_name, last_name, date_of_birth)
			VALUES (:fname, :lname, :dob)
		)~" };

		st.bind("fname", "Walter");
		st.bind("lname", "White");
		st.bind("dob", boost::gregorian::date{ 1958, 9, 7 });
		st.execute(); // prepares and executes

		st.bind("fname", "Schrader");
		st.bind("lname", "Hank");
		st.bind("dob", boost::gregorian::date{ 1966, 3, 1 });
		st.execute(); // only executes (with new parameter values)
	}
}

void run_recurring_statement_alt(connection& conn)
{
	auto st = conn.prepare(R"~(
			INSERT INTO person(first_name, last_name, date_of_birth)
			VALUES (:fname, :lname, :dob)
		)~");

	st.bind_execute({ { "fname", "Hank" }, { "lname", "Schrader" }, { "dob", boost::gregorian::date{ 1966, 3, 1 } } });
}

void run_recurring_statement_bind_ref(connection& conn)
{
	auto st = conn.prepare(R"~(
			INSERT INTO person(first_name, last_name, date_of_birth)
			VALUES (:fname, :lname, :dob)
		)~");

	std::string                           first{}, last{};
	std::optional<boost::gregorian::date> dob{};

	// Bind the parameters once.
	st.bind_ref("fname", first);
	st.bind_ref("lname", last);
	st.bind_ref("dob", dob);

	// Set values and execute.
	first = "Skyler";
	last  = "White";
	dob   = boost::gregorian::date{ 1970, 11, 8 };
	st.execute(); // prepares and executes

	// Set other values and execute again.
	first = "Jesse";
	last  = "Pinkman";
	dob   = std::nullopt;
	st.execute(); // only executes (with new parameter values)
}

void run_recurring_statement_bind_ref_alt(connection& conn)
{
	auto st = conn.prepare(R"~(
			INSERT INTO person(first_name, last_name, date_of_birth)
			VALUES (:fname, :lname, :dob)
		)~");

	std::string                           first{ "Skyler" }, last{ "White" };
	std::optional<boost::gregorian::date> dob = boost::gregorian::date{ 1970, 11, 8 };

	// Bind, prepare and execute
	st.bind_ref_execute({ { "fname", first }, { "lname", last }, { "dob", dob } });

	// Set other values and execute again.
	first = "Jesse";
	last  = "Pinkman";
	dob   = std::nullopt;
	st.execute(); // only executes (with new parameter values)
}

void catch_error()
{
	try
	{
		postgresql::connection bad_connection{ "host=does.not.exist" };
	}
	catch (const std::exception& e)
	{
		std::cerr << boost::diagnostic_information(e) << '\n';
	}
}

int main()
{
	try
	{
		//		connect_to_postgresql();
		//		connect_to_mysql();
		//		connect_to_sqlite();
		//		use_connection_pool();
		{
			sqlite::connection conn{ ":memory:" };
			run_non_parameterized_statements(conn);
			run_one_off_statements(conn);
			run_recurring_statement(conn);
			run_recurring_statement_alt(conn);
			run_recurring_statement_bind_ref(conn);
			run_recurring_statement_bind_ref_alt(conn);
			catch_error();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return 1;
	}
}
