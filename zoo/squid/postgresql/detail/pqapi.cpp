//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "pqapi.h"

namespace zoo {
namespace squid {
namespace postgresql {

pq_api pq_api::API{};

pq_api::pq_api()
{
}

pq_api::~pq_api()
{
}

void pq_api::clear(PGresult* res)
{
	return PQclear(res);
}

const char* pq_api::cmdTuples(PGresult* res)
{
	return PQcmdTuples(res);
}

PGconn* pq_api::connectdb(const char* conninfo)
{
	return PQconnectdb(conninfo);
}

int pq_api::isBusy(PGconn* conn)
{
	return PQisBusy(conn);
}

int pq_api::consumeInput(PGconn* conn)
{
	return PQconsumeInput(conn);
}

const char* pq_api::errorMessage(const PGconn* conn)
{
	return PQerrorMessage(conn);
}

PGresult* pq_api::exec(PGconn* conn, const char* query)
{
	return PQexec(conn, query);
}

PGresult* pq_api::execParams(PGconn*            conn,
                             const char*        command,
                             int                nParams,
                             const Oid*         paramTypes,
                             const char* const* paramValues,
                             const int*         paramLengths,
                             const int*         paramFormats,
                             int                resultFormat)
{
	return PQexecParams(conn, command, nParams, paramTypes, paramValues, paramLengths, paramFormats, resultFormat);
}

PGresult* pq_api::execPrepared(PGconn*            conn,
                               const char*        stmtName,
                               int                nParams,
                               const char* const* paramValues,
                               const int*         paramLengths,
                               const int*         paramFormats,
                               int                resultFormat)
{
	return PQexecPrepared(conn, stmtName, nParams, paramValues, paramLengths, paramFormats, resultFormat);
}

void pq_api::finish(PGconn* conn)
{
	return PQfinish(conn);
}

const char* pq_api::fname(const PGresult* res, int field_num)
{
	return PQfname(res, field_num);
}

void pq_api::freemem(void* ptr)
{
	return PQfreemem(ptr);
}

int pq_api::getisnull(const PGresult* res, int tup_num, int field_num)
{
	return PQgetisnull(res, tup_num, field_num);
}

const char* pq_api::getvalue(const PGresult* res, int tup_num, int field_num)
{
	return PQgetvalue(res, tup_num, field_num);
}

int pq_api::nfields(const PGresult* res)
{
	return PQnfields(res);
}

PGnotify* pq_api::notifies(PGconn* conn)
{
	return PQnotifies(conn);
}

int pq_api::ntuples(const PGresult* res)
{
	return PQntuples(res);
}

PGresult* pq_api::prepare(PGconn* conn, const char* stmtName, const char* query, int nParams, const Oid* paramTypes)
{
	return PQprepare(conn, stmtName, query, nParams, paramTypes);
}

void pq_api::reset(PGconn* conn)
{
	return PQreset(conn);
}

const char* pq_api::resStatus(ExecStatusType status)
{
	return PQresStatus(status);
}

const char* pq_api::resultErrorField(const PGresult* res, int fieldcode)
{
	return PQresultErrorField(res, fieldcode);
}

const char* pq_api::resultErrorMessage(const PGresult* res)
{
	return PQresultErrorMessage(res);
}

ExecStatusType pq_api::resultStatus(const PGresult* res)
{
	return PQresultStatus(res);
}

int pq_api::socket(const PGconn* conn)
{
	return PQsocket(conn);
}

ConnStatusType pq_api::status(const PGconn* conn)
{
	return PQstatus(conn);
}

int pq_api::setnonblocking(PGconn* conn, int arg)
{
	return PQsetnonblocking(conn, arg);
}

int pq_api::flush(PGconn* conn)
{
	return PQflush(conn);
}

PGresult* pq_api::getResult(PGconn* conn)
{
	return PQgetResult(conn);
}

int pq_api::sendQueryParams(PGconn*            conn,
                            const char*        command,
                            int                nParams,
                            const Oid*         paramTypes,
                            const char* const* paramValues,
                            const int*         paramLengths,
                            const int*         paramFormats,
                            int                resultFormat)
{
	return PQsendQueryParams(conn, command, nParams, paramTypes, paramValues, paramLengths, paramFormats, resultFormat);
}

int pq_api::sendPrepare(PGconn* conn, const char* stmtName, const char* query, int nParams, const Oid* paramTypes)
{
	return PQsendPrepare(conn, stmtName, query, nParams, paramTypes);
}

int pq_api::sendQueryPrepared(PGconn*            conn,
                              const char*        stmtName,
                              int                nParams,
                              const char* const* paramValues,
                              const int*         paramLengths,
                              const int*         paramFormats,
                              int                resultFormat)
{
	return PQsendQueryPrepared(conn, stmtName, nParams, paramValues, paramLengths, paramFormats, resultFormat);
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
