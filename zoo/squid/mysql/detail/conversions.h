//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/squid/core/types.h"

#include <mysql/mysql.h>

#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace zoo {
namespace squid {
namespace mysql {

void date_to_mysql_time(const date& in, MYSQL_TIME& out);

void time_of_day_to_mysql_time(const time_of_day& in, MYSQL_TIME& out);

void to_mysql_time(const date& in, MYSQL_TIME& out);

void to_mysql_time(const time_of_day& in, MYSQL_TIME& out);

void to_mysql_time(const time_point& in, MYSQL_TIME& out);

void boost_date_to_mysql_time(const boost::gregorian::date& in, MYSQL_TIME& out);

void boost_time_duration_to_mysql_time(const boost::posix_time::time_duration& in, MYSQL_TIME& out);

void to_mysql_time(const boost::gregorian::date& in, MYSQL_TIME& out);

void to_mysql_time(const boost::posix_time::time_duration& in, MYSQL_TIME& out);

void to_mysql_time(const boost::posix_time::ptime& in, MYSQL_TIME& out);

void from_mysql_time(const MYSQL_TIME& in, date& out);

void from_mysql_time(const MYSQL_TIME& in, time_of_day& out);

void from_mysql_time(const MYSQL_TIME& in, time_point& out);

void from_mysql_time(const MYSQL_TIME& in, boost::gregorian::date& out);

void from_mysql_time(const MYSQL_TIME& in, boost::posix_time::time_duration& out);

void from_mysql_time(const MYSQL_TIME& in, boost::posix_time::ptime& out);

} // namespace mysql
} // namespace squid
} // namespace zoo
