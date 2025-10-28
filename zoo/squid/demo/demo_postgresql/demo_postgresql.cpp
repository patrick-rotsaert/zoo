//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/connection.h"
#include "zoo/squid/postgresql/tuple.h"
#include "zoo/squid/demo/demo_common/demo_common.h"

#include <boost/asio/io_context.hpp>

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace zoo {
namespace squid {
namespace demo {

void async_exec_demo(postgresql::connection& connection)
{
	boost::asio::io_context io{};

	connection.async_exec(
	    io, "SELECT :one AS one, :pi AS pi", { { "one", 1 }, { "pi", 3.141592 } }, [](postgresql::async_exec_result result) {
		    if (std::holds_alternative<postgresql::async_error>(result))
		    {
			    const auto& err = std::get<postgresql::async_error>(result);
			    std::cerr << err.format() << '\n';
		    }
		    else
		    {
			    const auto& rs = std::get<postgresql::resultset>(result);
			    std::cout << "Affected rows: " << rs.affected_rows() << '\n';
			    std::cout << "Returned rows: " << rs.size() << '\n';
			    for (const auto& tup : rs)
			    {
				    for (const auto& field : tup)
				    {
					    std::cout << "Field " << field.name() << " = " << field.to_string_view() << '\n';
				    }
				    std::cout << "one is " << tup["one"].to_int() << ", pi is " << tup["pi"].to_double() << '\n';
			    }
			    const auto& tup = *(rs.end() - 1);
			    std::cout << "one is " << tup["one"].to_int() << ", pi is " << tup["pi"].to_double() << '\n';
		    }
	    });

	io.run();
}

void async_prepare_demo(postgresql::connection& connection)
{
	boost::asio::io_context io{};

	connection.async_prepare(io, "SELECT :one AS one, :pi AS pi", [](postgresql::async_prepare_result result) {
		if (std::holds_alternative<postgresql::async_error>(result))
		{
			const auto& err = std::get<postgresql::async_error>(result);
			std::cerr << err.format() << '\n';
		}
		else
		{
			auto prepared = std::get<std::shared_ptr<postgresql::async_prepared_statement>>(result);
			prepared->async_exec({ { "one", 1 }, { "pi", 3.141592 } }, [prepared](postgresql::async_exec_result result) {
				// The prepared statement must not be deleted before the handler returns, that's what the capture is for.
				(void)prepared;

				if (std::holds_alternative<postgresql::async_error>(result))
				{
					const auto& err = std::get<postgresql::async_error>(result);
					std::cerr << err.format() << '\n';
				}
				else
				{
					const auto& rs = std::get<postgresql::resultset>(result);
					std::cout << "Affected rows: " << rs.affected_rows() << '\n';
					std::cout << "Returned rows: " << rs.size() << '\n';
					for (const auto& tup : rs)
					{
						for (const auto& field : tup)
						{
							std::cout << "Field " << field.name() << " = " << field.to_string_view() << '\n';
						}
						std::cout << "one is " << tup["one"].to_int() << ", pi is " << tup["pi"].to_double() << '\n';
					}
					const auto& tup = *(rs.end() - 1);
					std::cout << "one is " << tup["one"].to_int() << ", pi is " << tup["pi"].to_double() << '\n';
				}
			});
		}
	});

	io.run();
}

void demo()
{
	constexpr auto connection_info = "host=localhost port=54321 dbname=squid_demo_postgresql user=postgres password=Pass123";

	postgresql::connection connection{ connection_info };
	std::cout << "opened database " << std::quoted(connection_info) << "\n";

	demo_all(connection, Backend::POSTGRESQL);
	async_exec_demo(connection);
	async_prepare_demo(connection);
}

} // namespace demo
} // namespace squid
} // namespace zoo

int main()
{
	// spdlog::set_level(spdlog::level::trace);
	// spdlog::set_pattern("%L [%Y-%m-%d %H:%M:%S.%f](%t) %^%v%$ [%s:%#]");

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
