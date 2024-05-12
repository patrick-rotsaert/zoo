# SQUID - C++ Database Access Library

## Quick start

All code samples imply:
```cpp
using namespace zoo::squid;
```

### Connecting to a database

#### PostgreSQL

Pass the connection parameter string to the constructor of `postgresql::connection`.
Internally, the connection string is passed unaltered to [PQconnectdb](https://www.postgresql.org/docs/current/libpq-connect.html#LIBPQ-PQCONNECTDB).

```cpp
#include "zoo/squid/postgresql/connection.h"

void connect_to_postgresql()
{
	constexpr auto connection_info = "host=localhost port=54321 dbname=quickstart user=postgres password=Pass123";

	postgresql::connection connection{ connection_info };
	std::cout << "opened database " << std::quoted(connection_info) << "\n";
}
```

Link the required library in CMake
```cmake
target_link_libraries(my_app PRIVATE zoo::squid_postgresql)
```

#### MySQL

Pass the connection parameter string to the constructor of `mysql::connection`.
Internally, the connection string is parsed and translated to a series of [mysql_options](https://dev.mysql.com/doc/c-api/8.0/en/mysql-options.html) calls and one [mysql_real_connect](https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-connect.html) call.

```cpp
#include "zoo/squid/mysql/connection.h"

void connect_to_mysql()
{
	constexpr auto connection_info = R"~(host=127.0.0.1 port=13306 db=quickstart user=myuser passwd="Pass 123")~";

	mysql::connection connection{ connection_info };
	std::cout << "opened database " << std::quoted(connection_info) << "\n";
}
```

Parameters that can occur in the connection string are:
- `host`: The MySQL server host. See `host` parameter of [mysql_real_connect](https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-connect.html).
- `user`: The user name. See `user` parameter of [mysql_real_connect](https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-connect.html).
- `passwd`: The password. See `passwd` parameter of [mysql_real_connect](https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-connect.html).
- `db`: The database name. See `db` parameter of [mysql_real_connect](https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-connect.html).
- `port`: The TCP port. Must be convertible to `unsigned int`. See `port` parameter of [mysql_real_connect](https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-connect.html).
- `unix_socket`: The unix domain socket. See `unix_socket` parameter of [mysql_real_connect](https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-connect.html).
- `charset`: The name of the character set to use as the default character set. See parameter `MYSQL_SET_CHARSET_NAME` of [mysql_options](https://dev.mysql.com/doc/c-api/8.0/en/mysql-options.html).
- `sslca`: The path name of the Certificate Authority (CA) certificate file. See parameter `MYSQL_OPT_SSL_CA` of [mysql_options](https://dev.mysql.com/doc/c-api/8.0/en/mysql-options.html).
- `sslcert`: The path name of the client public key certificate file. See parameter `MYSQL_OPT_SSL_CERT` of [mysql_options](https://dev.mysql.com/doc/c-api/8.0/en/mysql-options.html).
- `sslkey`: The path name of the client private key file. See parameter `MYSQL_OPT_SSL_KEY` of [mysql_options](https://dev.mysql.com/doc/c-api/8.0/en/mysql-options.html).


Parameter values may be quoted if necessary. See `passwd` in the sample above

Link the required library in CMake
```cmake
target_link_libraries(my_app PRIVATE zoo::squid_mysql)
```

#### SQLite

Pass the path of the database file to the constructor of `sqlite::connection`.
Internally, the filename is passed unaltered to [sqlite3_open](https://www.sqlite.org/c3ref/open.html).

```cpp
#include "zoo/squid/sqlite3/connection.h"

void connect_to_sqlite()
{
	constexpr auto path = "quickstart.db";
	// constexpr auto path = ":memory:";

	sqlite::connection connection{ path };
	std::cout << "opened database " << std::quoted(path) << "\n";
}
```

Link the required library in CMake
```cmake
target_link_libraries(my_app PRIVATE zoo::squid_sqlite)
```

### Using a connection pool

```cpp
#include "zoo/squid/core/connectionpool.h"
#include "zoo/squid/core/connection.h"
#include "zoo/squid/sqlite3/backendconnectionfactory.h"

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
```

### Executing non-parameterized statements

The `connection::execute` method executes a single statement without parameter nor result bindings.
This is typically used for schema updates. All statements can be executed but the method does not return a result set.

```cpp
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
```

### Executing parameterized statements

This is the recommended way of executing statements when values are only known at runtime.
It protects against SQL injection and avoids the need for escaping string values.

There are two statement classes:

1. For one-off statements, use the `statement` class.
2. For recurring statements, use the `prepared_statement` class.

Both are derived from `basic_statement` which provides the methods for parameter and result binding,
so their use is practically the same.

#### One-off statements

```cpp
#include "zoo/squid/core/statement.h"

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
```

Each call to `statement::execute` will be treated as a new query.

#### Recurring statements

```cpp
#include "zoo/squid/core/preparedstatement.h"

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
```

Parameter values can also be bound by reference.
For subsequent statement execution, the parameters don't need to be bound again, it is sufficient to update the bound values.

```cpp
void run_recurring_statement_bind_ref(connection& conn)
{
  prepared_statement st{ conn, R"~(
    INSERT INTO person(first_name, last_name, date_of_birth)
    VALUES (:fname, :lname, :dob)
  )~" };

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
```

### Parameter and result binding

For more information about parameter and result bindig, please refer to the comments in [basicstatement.h](core/basicstatement.h).

### Errors

This library throws exceptions in case of any error.
Exceptions are thrown with `BOOST_THROW_EXCEPTION`, so it is possible to get diagnostics information from a caught exception.

```cpp
#include "zoo/squid/postgresql/connection.h"
#include <boost/exception/diagnostic_information.hpp>

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
```

## Motivation

This library was somewhat inspired by the [SOCI](https://github.com/SOCI/soci) library, which I have used for a long time.
SOCI is a great and powerful library but some of its design choices, undoubtedly chosen to make life easier, make it hard to use at times:
* Streamed statements: The actual statement preparation or execution happens in class destructors.
  Without very good care, this can cause the application to abort in case of exceptions.
  In this library, destructors never throw.
* Query parameters are always bound by reference (soci::use).
  This can give unexpected results when passed temporaries.
  The SOCI documentation warns about this, but still it is easy to overlook and the compiler does not warn you.
  This library requires you to explicitly state if you bind a parameter by value or by reference.

However, I did like the SOCI architecture where the library user interacts mainly with a database-agnostic
frontend which passes on the work to a database-specific backend.
I gladly borrowed that idea but, apart from that, this a totally different implementation.

For those wondering... _SQUID_ is an acronym for **SQ**L: **U**nified **I**nterface to **D**atabases.

## Database clients

Currently supported databases are:

* PostgreSQL
* SQLite3
* MySQL

Future planned:
* ODBC
* Oracle

## TODO list

- [-] Write unit tests.
   - [ ] PostgreSQL
   - [ ] MySQL
   - [x] SQLite
- [ ] Write quick start.
- [x] Add support for binding Boost.DateTime types.
- [ ] Add support for binding Howard Hinnant date types.
- [ ] Add support for binding Boost descibe'd structs.
- [ ] Add support for ODBC.
- [ ] Add support for Oracle.
- [x] Add connection pool.
- [x] Add transaction class.
- [x] Bind parameters by reference.
- [x] Add logging and support custom logging backend. See [../common/logging](../common/logging).
- [ ] Add support for string encodings / charsets, std::wstring and wchar_t.
