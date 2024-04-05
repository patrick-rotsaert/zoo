//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/mysql/connection.h"
#include "zoo/squid/demo/demo_common/demo_common.h"

#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace zoo {
namespace squid {
namespace demo {

void demo()
{
	constexpr auto connection_info = "host=127.0.0.1 port=13306 db=squid_demo_mysql user=myuser passwd=Pass123";

	mysql::connection connection{ connection_info };
	std::cout << "opened database " << std::quoted(connection_info) << "\n";

	demo_all(connection, Backend::MYSQL);
}

} // namespace demo
} // namespace squid
} // namespace zoo

int main()
{
	try
	{
		zoo::squid::demo::demo();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return 1;
	}
}
