//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <libpq-fe.h>

namespace zoo {
namespace squid {
namespace postgresql {

class ipq_api
{
public:
	ipq_api();
	virtual ~ipq_api();

	ipq_api(ipq_api&&)      = delete;
	ipq_api(const ipq_api&) = delete;

	ipq_api& operator=(ipq_api&&)      = delete;
	ipq_api& operator=(const ipq_api&) = delete;

	virtual void           clear(PGresult* res)                                                                                   = 0;
	virtual const char*    cmdTuples(PGresult* res)                                                                               = 0;
	virtual PGconn*        connectdb(const char* conninfo)                                                                        = 0;
	virtual int            isBusy(PGconn* conn)                                                                                   = 0;
	virtual int            consumeInput(PGconn* conn)                                                                             = 0;
	virtual const char*    errorMessage(const PGconn* conn)                                                                       = 0;
	virtual PGresult*      exec(PGconn* conn, const char* query)                                                                  = 0;
	virtual PGresult*      execParams(PGconn*            conn,
	                                  const char*        command,
	                                  int                nParams,
	                                  const Oid*         paramTypes,
	                                  const char* const* paramValues,
	                                  const int*         paramLengths,
	                                  const int*         paramFormats,
	                                  int                resultFormat)                                                                           = 0;
	virtual PGresult*      execPrepared(PGconn*            conn,
	                                    const char*        stmtName,
	                                    int                nParams,
	                                    const char* const* paramValues,
	                                    const int*         paramLengths,
	                                    const int*         paramFormats,
	                                    int                resultFormat)                                                                         = 0;
	virtual void           finish(PGconn* conn)                                                                                   = 0;
	virtual const char*    fname(const PGresult* res, int field_num)                                                              = 0;
	virtual void           freemem(void* ptr)                                                                                     = 0;
	virtual int            getisnull(const PGresult* res, int tup_num, int field_num)                                             = 0;
	virtual const char*    getvalue(const PGresult* res, int tup_num, int field_num)                                              = 0;
	virtual int            nfields(const PGresult* res)                                                                           = 0;
	virtual PGnotify*      notifies(PGconn* conn)                                                                                 = 0;
	virtual int            ntuples(const PGresult* res)                                                                           = 0;
	virtual PGresult*      prepare(PGconn* conn, const char* stmtName, const char* query, int nParams, const Oid* paramTypes)     = 0;
	virtual void           reset(PGconn* conn)                                                                                    = 0;
	virtual const char*    resStatus(ExecStatusType status)                                                                       = 0;
	virtual const char*    resultErrorField(const PGresult* res, int fieldcode)                                                   = 0;
	virtual const char*    resultErrorMessage(const PGresult* res)                                                                = 0;
	virtual ExecStatusType resultStatus(const PGresult* res)                                                                      = 0;
	virtual int            socket(const PGconn* conn)                                                                             = 0;
	virtual ConnStatusType status(const PGconn* conn)                                                                             = 0;
	virtual int            setnonblocking(PGconn* conn, int arg)                                                                  = 0;
	virtual int            flush(PGconn* conn)                                                                                    = 0;
	virtual PGresult*      getResult(PGconn* conn)                                                                                = 0;
	virtual int            sendQueryParams(PGconn*            conn,
	                                       const char*        command,
	                                       int                nParams,
	                                       const Oid*         paramTypes,
	                                       const char* const* paramValues,
	                                       const int*         paramLengths,
	                                       const int*         paramFormats,
	                                       int                resultFormat)                                                                      = 0;
	virtual int            sendPrepare(PGconn* conn, const char* stmtName, const char* query, int nParams, const Oid* paramTypes) = 0;
	virtual int            sendQueryPrepared(PGconn*            conn,
	                                         const char*        stmtName,
	                                         int                nParams,
	                                         const char* const* paramValues,
	                                         const int*         paramLengths,
	                                         const int*         paramFormats,
	                                         int                resultFormat)                                                                    = 0;
};

} // namespace postgresql
} // namespace squid
} // namespace zoo
