//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "ipqapi.h"

namespace zoo {
namespace squid {
namespace postgresql {

class pq_api final : public ipq_api
{
public:
	static pq_api API;

	pq_api();
	~pq_api() override;

	pq_api(pq_api&&)      = delete;
	pq_api(const pq_api&) = delete;

	pq_api& operator=(pq_api&&)      = delete;
	pq_api& operator=(const pq_api&) = delete;

	void           clear(PGresult* res) override;
	const char*    cmdTuples(PGresult* res) override;
	PGconn*        connectdb(const char* conninfo) override;
	int            isBusy(PGconn* conn) override;
	int            consumeInput(PGconn* conn) override;
	const char*    errorMessage(const PGconn* conn) override;
	PGresult*      exec(PGconn* conn, const char* query) override;
	PGresult*      execParams(PGconn*            conn,
	                          const char*        command,
	                          int                nParams,
	                          const Oid*         paramTypes,
	                          const char* const* paramValues,
	                          const int*         paramLengths,
	                          const int*         paramFormats,
	                          int                resultFormat) override;
	PGresult*      execPrepared(PGconn*            conn,
	                            const char*        stmtName,
	                            int                nParams,
	                            const char* const* paramValues,
	                            const int*         paramLengths,
	                            const int*         paramFormats,
	                            int                resultFormat) override;
	void           finish(PGconn* conn) override;
	const char*    fname(const PGresult* res, int field_num) override;
	void           freemem(void* ptr) override;
	int            getisnull(const PGresult* res, int tup_num, int field_num) override;
	const char*    getvalue(const PGresult* res, int tup_num, int field_num) override;
	int            nfields(const PGresult* res) override;
	PGnotify*      notifies(PGconn* conn) override;
	int            ntuples(const PGresult* res) override;
	PGresult*      prepare(PGconn* conn, const char* stmtName, const char* query, int nParams, const Oid* paramTypes) override;
	void           reset(PGconn* conn) override;
	const char*    resStatus(ExecStatusType status) override;
	const char*    resultErrorField(const PGresult* res, int fieldcode) override;
	const char*    resultErrorMessage(const PGresult* res) override;
	ExecStatusType resultStatus(const PGresult* res) override;
	int            socket(const PGconn* conn) override;
	ConnStatusType status(const PGconn* conn) override;
	int            setnonblocking(PGconn* conn, int arg) override;
	int            flush(PGconn* conn) override;
	PGresult*      getResult(PGconn* conn) override;
	int            sendQueryParams(PGconn*            conn,
	                               const char*        command,
	                               int                nParams,
	                               const Oid*         paramTypes,
	                               const char* const* paramValues,
	                               const int*         paramLengths,
	                               const int*         paramFormats,
	                               int                resultFormat) override;
	int            sendPrepare(PGconn* conn, const char* stmtName, const char* query, int nParams, const Oid* paramTypes) override;
	int            sendQueryPrepared(PGconn*            conn,
	                                 const char*        stmtName,
	                                 int                nParams,
	                                 const char* const* paramValues,
	                                 const int*         paramLengths,
	                                 const int*         paramFormats,
	                                 int                resultFormat) override;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
