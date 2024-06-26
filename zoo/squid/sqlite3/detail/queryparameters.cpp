//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "queryparameters.h"

#include "zoo/squid/sqlite3/error.h"
#include "zoo/squid/sqlite3/detail/isqliteapi.h"

#include "zoo/common/conversion/conversion.h"
#include "zoo/common/misc/always_false.hpp"
#include "zoo/common/misc/throw_exception.h"

#include <sqlite3.h>

#include <iomanip>
#include <cassert>

#include <sqlite3.h>

namespace zoo {
namespace squid {
namespace sqlite {

namespace {

void bind_parameter(isqlite_api& api, sqlite3& connection, sqlite3_stmt& statement, const std::string& name, const parameter& parameter)
{
	auto tmp_name        = ":" + name;
	auto parameter_index = api.bind_parameter_index(&statement, tmp_name.c_str());
	if (parameter_index < 1)
	{
		// hacky!
		tmp_name.front() = '@';
		parameter_index  = api.bind_parameter_index(&statement, tmp_name.c_str());
		if (parameter_index < 1)
		{
			// hacky again ;)
			tmp_name.front() = '$';
			parameter_index  = api.bind_parameter_index(&statement, tmp_name.c_str());
			if (parameter_index < 1)
			{
				std::ostringstream msg;
				msg << "Parameter name " << std::quoted(name) << " was not found in the statement";
				ZOO_THROW_EXCEPTION(error{ msg.str() });
			}
		}
	}
	assert(parameter_index > 0);

#define BIND0(f)                                                                                                                           \
	do                                                                                                                                     \
	{                                                                                                                                      \
		if (SQLITE_OK != f(&statement, parameter_index))                                                                                   \
		{                                                                                                                                  \
			ZOO_THROW_EXCEPTION(error(api, #f " failed", connection));                                                                     \
		}                                                                                                                                  \
	} while (false)

#define BIND(f, ...)                                                                                                                       \
	do                                                                                                                                     \
	{                                                                                                                                      \
		if (SQLITE_OK != f(&statement, parameter_index, __VA_ARGS__))                                                                      \
		{                                                                                                                                  \
			ZOO_THROW_EXCEPTION(error(api, #f " failed", connection));                                                                     \
		}                                                                                                                                  \
	} while (false)

	std::visit(
	    [&api, &connection, &statement, &parameter_index](auto&& arg) {
		    using T = std::decay_t<decltype(arg)>;
		    if constexpr (std::is_same_v<T, const std::nullopt_t*>)
		    {
			    BIND0(api.bind_null);
		    }
		    else if constexpr (std::is_same_v<T, const bool*>)
		    {
			    BIND(api.bind_int, *arg ? 1 : 0);
		    }
		    else if constexpr (std::is_same_v<T, const char*>)
		    {
			    BIND(api.bind_text, arg, 1, SQLITE_STATIC);
		    }
		    else if constexpr (std::is_same_v<T, const signed char*> || std::is_same_v<T, const unsigned char*> ||
		                       std::is_same_v<T, const std::int16_t*> || std::is_same_v<T, const std::uint16_t*> ||
		                       std::is_same_v<T, const std::int32_t*>)
		    {
			    BIND(api.bind_int, static_cast<int>(*arg));
		    }
		    else if constexpr (std::is_same_v<T, const std::uint32_t*> || std::is_same_v<T, const std::int64_t*> ||
		                       std::is_same_v<T, const std::uint64_t*>)
		    {
			    BIND(api.bind_int64, static_cast<sqlite3_int64>(*arg));
		    }
		    else if constexpr (std::is_same_v<T, const float*> || std::is_same_v<T, const double*> || std::is_same_v<T, const long double*>)
		    {
			    BIND(api.bind_double, static_cast<double>(*arg));
		    }
		    else if constexpr (std::is_same_v<T, const std::string*> || std::is_same_v<T, const std::string_view*>)
		    {
			    BIND(api.bind_text, arg->data(), static_cast<int>(arg->length()), SQLITE_STATIC);
		    }
		    else if constexpr (std::is_same_v<T, const byte_string*> || std::is_same_v<T, const byte_string_view*>)
		    {
			    BIND(api.bind_blob, arg->data(), static_cast<int>(arg->length()), SQLITE_STATIC);
		    }
		    else if constexpr (std::is_same_v<T, const time_point*>)
		    {
			    auto tmp = conversion::time_point_to_sql(*arg);
			    BIND(api.bind_text, tmp.data(), static_cast<int>(tmp.length()), SQLITE_TRANSIENT);
		    }
		    else if constexpr (std::is_same_v<T, const date*>)
		    {
			    auto tmp = conversion::date_to_string(*arg);
			    BIND(api.bind_text, tmp.data(), static_cast<int>(tmp.length()), SQLITE_TRANSIENT);
		    }
		    else if constexpr (std::is_same_v<T, const time_of_day*>)
		    {
			    auto tmp = conversion::time_of_day_to_string(*arg);
			    BIND(api.bind_text, tmp.data(), static_cast<int>(tmp.length()), SQLITE_TRANSIENT);
		    }
		    else if constexpr (std::is_same_v<T, const boost::posix_time::ptime*>)
		    {
			    auto tmp = conversion::boost_ptime_to_sql(*arg);
			    BIND(api.bind_text, tmp.data(), static_cast<int>(tmp.length()), SQLITE_TRANSIENT);
		    }
		    else if constexpr (std::is_same_v<T, const boost::gregorian::date*>)
		    {
			    auto tmp = conversion::boost_date_to_string(*arg);
			    BIND(api.bind_text, tmp.data(), static_cast<int>(tmp.length()), SQLITE_TRANSIENT);
		    }
		    else if constexpr (std::is_same_v<T, const boost::posix_time::time_duration*>)
		    {
			    auto tmp = conversion::boost_time_duration_to_string(*arg);
			    BIND(api.bind_text, tmp.data(), static_cast<int>(tmp.length()), SQLITE_TRANSIENT);
		    }
		    else
		    {
			    static_assert(always_false_v<T>, "non-exhaustive visitor!");
		    }
	    },
	    parameter.pointer());
}

} // namespace

/*static*/ void
query_parameters::bind(isqlite_api& api, sqlite3& connection, sqlite3_stmt& statement, const std::map<std::string, parameter>& parameters)
{
	for (const auto& pair : parameters)
	{
		bind_parameter(api, connection, statement, pair.first, pair.second);
	}
}

} // namespace sqlite
} // namespace squid
} // namespace zoo
