//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "pqapimock.h"

namespace zoo {
namespace squid {
namespace postgresql {

namespace {

PGconn   g_connection;
PGresult g_result;

} // namespace

PGconn*   pq_api_mock::test_connection = &g_connection;
PGresult* pq_api_mock::test_result  = &g_result;

std::shared_ptr<PGconn>   pq_api_mock::test_connection_shared = std::make_shared<PGconn>();
std::shared_ptr<PGresult> pq_api_mock::test_result_shared  = std::make_shared<PGresult>();

pq_api_mock::pq_api_mock()
{
}

pq_api_mock::~pq_api_mock()
{
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
