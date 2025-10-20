//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "ipqapi.h"

#include <memory>
#include <gmock/gmock.h>

struct pg_conn
{
};

struct pg_result
{
};

namespace zoo {
namespace squid {
namespace postgresql {

class pq_api_mock : public ipq_api
{
public:
	static PGconn*   test_connection;
	static PGresult* test_result;

	static std::shared_ptr<PGconn>   test_connection_shared;
	static std::shared_ptr<PGresult> test_result_shared;

	pq_api_mock();
	~pq_api_mock() override;

	pq_api_mock(pq_api_mock&&)      = delete;
	pq_api_mock(const pq_api_mock&) = delete;

	pq_api_mock& operator=(pq_api_mock&&)      = delete;
	pq_api_mock& operator=(const pq_api_mock&) = delete;

	MOCK_METHOD(void, clear, (PGresult * res), (override));
	MOCK_METHOD(const char*, cmdTuples, (PGresult * res), (override));
	MOCK_METHOD(PGconn*, connectdb, (const char* conninfo), (override));
	MOCK_METHOD(int, isBusy, (PGconn * conn), (override));
	MOCK_METHOD(int, consumeInput, (PGconn * conn), (override));
	MOCK_METHOD(const char*, errorMessage, (const PGconn* conn), (override));
	MOCK_METHOD(PGresult*, exec, (PGconn * conn, const char* query), (override));
	MOCK_METHOD(PGresult*,
	            execParams,
	            (PGconn * conn,
	             const char*        command,
	             int                nParams,
	             const Oid*         paramTypes,
	             const char* const* paramValues,
	             const int*         paramLengths,
	             const int*         paramFormats,
	             int                resultFormat),
	            (override));
	MOCK_METHOD(PGresult*,
	            execPrepared,
	            (PGconn * conn,
	             const char*        stmtName,
	             int                nParams,
	             const char* const* paramValues,
	             const int*         paramLengths,
	             const int*         paramFormats,
	             int                resultFormat),
	            (override));
	MOCK_METHOD(void, finish, (PGconn * conn), (override));
	MOCK_METHOD(const char*, fname, (const PGresult* res, int field_num), (override));
	MOCK_METHOD(void, freemem, (void* ptr), (override));
	MOCK_METHOD(int, getisnull, (const PGresult* res, int tup_num, int field_num), (override));
	MOCK_METHOD(const char*, getvalue, (const PGresult* res, int tup_num, int field_num), (override));
	MOCK_METHOD(int, nfields, (const PGresult* res), (override));
	MOCK_METHOD(PGnotify*, notifies, (PGconn * conn), (override));
	MOCK_METHOD(int, ntuples, (const PGresult* res), (override));
	MOCK_METHOD(PGresult*,
	            prepare,
	            (PGconn * conn, const char* stmtName, const char* query, int nParams, const Oid* paramTypes),
	            (override));
	MOCK_METHOD(void, reset, (PGconn * conn), (override));
	MOCK_METHOD(const char*, resStatus, (ExecStatusType status), (override));
	MOCK_METHOD(const char*, resultErrorField, (const PGresult* res, int fieldcode), (override));
	MOCK_METHOD(const char*, resultErrorMessage, (const PGresult* res), (override));
	MOCK_METHOD(ExecStatusType, resultStatus, (const PGresult* res), (override));
	MOCK_METHOD(int, socket, (const PGconn* conn), (override));
	MOCK_METHOD(ConnStatusType, status, (const PGconn* conn), (override));
	MOCK_METHOD(int, setnonblocking, (PGconn * conn, int arg), (override));
	MOCK_METHOD(int, flush, (PGconn * conn), (override));
	MOCK_METHOD(PGresult*, getResult, (PGconn * conn), (override));
	MOCK_METHOD(int,
	            sendQueryParams,
	            (PGconn * conn,
	             const char*        command,
	             int                nParams,
	             const Oid*         paramTypes,
	             const char* const* paramValues,
	             const int*         paramLengths,
	             const int*         paramFormats,
	             int                resultFormat),
	            (override));
};

using pq_api_mock_nice   = testing::NiceMock<pq_api_mock>;
using pq_api_mock_naggy  = testing::NaggyMock<pq_api_mock>;
using pq_api_mock_strict = testing::StrictMock<pq_api_mock>;

} // namespace postgresql
} // namespace squid
} // namespace zoo

// This regex search and replace patterns can help to convert declarations into MOCK_METHOD macro calls
// Search pattern: ^(.+)\s([a-zA-Z_][a-zA-Z0-9_]*)\s*(\([^)]*\))\s*override\s*;\s*$
// Replace pattern: MOCK_METHOD(\1,\2,\3,(override));
