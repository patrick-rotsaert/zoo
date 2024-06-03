//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/statement.h"
#include "zoo/squid/postgresql/error.h"

#include "zoo/squid/postgresql/detail/ipqapi.h"
#include "zoo/squid/postgresql/detail/query.h"
#include "zoo/squid/postgresql/detail/queryparameters.h"
#include "zoo/squid/postgresql/detail/queryresults.h"
#include "zoo/squid/postgresql/detail/connectionchecker.h"
#include "zoo/squid/postgresql/detail/execresult.h"

#include "zoo/common/conversion/conversion.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/throw_exception.h"

#include <optional>
#include <atomic>
#include <cassert>

namespace zoo {
namespace squid {
namespace postgresql {

namespace {

std::string next_statement_name()
{
	static std::atomic<uint64_t> statement_number = 0;
	return std::string{ "s_" } + std::to_string(++statement_number);
}

} // namespace

class statement::impl final
{
	ipq_api*                          api_;
	std::shared_ptr<PGconn>           connection_;
	std::unique_ptr<postgresql_query> query_;
	bool                              reuse_statement_;
	bool                              prepared_;
	std::optional<std::string>        stmt_name_;
	std::optional<exec_result>        exec_result_;
	std::unique_ptr<query_results>    query_results_;

public:
	explicit impl(ipq_api* api, std::shared_ptr<PGconn> connection, std::string_view query, bool reuse_statement)
	    : api_{ api }
	    , connection_{ std::move(connection) }
	    , query_{ std::make_unique<postgresql_query>(query) }
	    , reuse_statement_{ reuse_statement }
	    , prepared_{}
	    , stmt_name_{}
	    , exec_result_{}
	    , query_results_{}
	{
		assert(this->connection_);
	}

	~impl() noexcept
	{
		try
		{
			if (this->prepared_)
			{
				assert(this->stmt_name_);
				std::shared_ptr<PGresult>{ this->api_->exec(connection_checker::check(this->api_, this->connection_),
					                                        ("DEALLOCATE " + this->stmt_name_.value()).c_str()),
					                       [this](PGresult* res) { this->api_->clear(res); } };
			}
		}
		catch (...)
		{
			;
		}
	}

	template<typename ResultsContainer>
	void set_exec_result(std::shared_ptr<PGresult> pgresult, std::string_view exec_function, const ResultsContainer& results)
	{
		if (pgresult)
		{
			const auto status = this->api_->resultStatus(pgresult.get());
			if (PGRES_TUPLES_OK == status)
			{
				this->exec_result_ = exec_result{ .pgresult = pgresult, .rows = this->api_->ntuples(pgresult.get()), .current_row = 0 };
			}
			else if (PGRES_COMMAND_OK == status)
			{
				this->exec_result_ = exec_result{ .pgresult = pgresult, .rows = 0, .current_row = 0 };
			}
			else
			{
				ZOO_THROW_EXCEPTION(error{ this->api_, std::string{ exec_function } + " failed", *this->connection_, *pgresult.get() });
			}

			this->query_results_ = std::make_unique<query_results>(this->api_, pgresult, results);
		}
		else
		{
			ZOO_THROW_EXCEPTION(error{ this->api_, std::string{ exec_function } + " failed", *this->connection_ });
		}
	}

	template<typename ResultsContainer>
	void execute(const std::map<std::string, parameter>& parameters, const ResultsContainer& results)
	{
		this->exec_result_ = std::nullopt;
		this->query_results_.reset();

		query_parameters query_params{ *this->query_, parameters };

		assert(query_params.parameter_count() == this->query_->parameter_count());

		if (this->reuse_statement_)
		{
			if (!this->prepared_)
			{
				if (!this->stmt_name_)
				{
					this->stmt_name_ = next_statement_name();
				}

				ZOO_LOG(trace, "preparing: {}", this->query_->query());

				std::shared_ptr<PGresult> pgresult{ this->api_->prepare(connection_checker::check(this->api_, this->connection_),
					                                                    this->stmt_name_->c_str(),
					                                                    this->query_->query().c_str(),
					                                                    this->query_->parameter_count(),
					                                                    nullptr),
					                                [this](PGresult* res) { this->api_->clear(res); } };
				if (pgresult)
				{
					auto status = this->api_->resultStatus(pgresult.get());
					if (PGRES_COMMAND_OK != status)
					{
						ZOO_THROW_EXCEPTION(error{ this->api_, "PQprepare failed", *this->connection_, *pgresult });
					}
					this->prepared_ = true;
				}
				else
				{
					ZOO_THROW_EXCEPTION(error{ this->api_, "PQprepare failed", *this->connection_ });
				}
			}

			assert(this->stmt_name_);

			this->set_exec_result(
			    std::shared_ptr<PGresult>{ this->api_->execPrepared(connection_checker::check(this->api_, this->connection_),
			                                                        this->stmt_name_->c_str(),
			                                                        query_params.parameter_count(),
			                                                        query_params.parameter_values(),
			                                                        nullptr,
			                                                        nullptr,
			                                                        0),
			                               [this](PGresult* res) { this->api_->clear(res); } },
			    "PQexecPrepared",
			    results);
		}
		else
		{
			this->set_exec_result(
			    std::shared_ptr<PGresult>{ this->api_->execParams(connection_checker::check(this->api_, this->connection_),
			                                                      this->query_->query().c_str(),
			                                                      query_params.parameter_count(),
			                                                      nullptr,
			                                                      query_params.parameter_values(),
			                                                      nullptr,
			                                                      nullptr,
			                                                      0),
			                               [this](PGresult* res) { this->api_->clear(res); } },
			    "PQexecParams",
			    results);
		}
	}

	bool fetch()
	{
		if (!this->exec_result_ || !this->query_results_)
		{
			ZOO_THROW_EXCEPTION(error{ "Cannot fetch tuple from a statement that has not been executed" });
		}

		auto& exec_result = this->exec_result_.value();

		if (exec_result.current_row == exec_result.rows)
		{
			return false;
		}

		this->query_results_->fetch(exec_result.current_row++);

		return true;
	}

	std::size_t field_count()
	{
		if (this->query_results_)
		{
			return this->query_results_->field_count();
		}
		else
		{
			ZOO_THROW_EXCEPTION(error{ "Cannot get field count from a statement that has not been executed" });
		}
	}

	std::string field_name(std::size_t index)
	{
		if (this->query_results_)
		{
			return this->query_results_->field_name(index);
		}
		else
		{
			ZOO_THROW_EXCEPTION(error{ "Cannot get field name from a statement that has not been executed" });
		}
	}

	std::uint64_t affected_rows()
	{
		if (this->exec_result_)
		{
			const auto num = this->api_->cmdTuples(this->exec_result_->pgresult.get());
			if (!num || !(*num))
			{
				return 0ull;
			}
			return conversion::string_to_number<std::uint64_t>(num);
		}
		else
		{
			ZOO_THROW_EXCEPTION(error{ "Cannot get the number of affected rows from a statement that has not been executed" });
		}
	}
};

statement::statement(ipq_api* api, std::shared_ptr<PGconn> connection, std::string_view query, bool reuse_statement)
    : ibackend_statement{}
    , pimpl_{ std::make_unique<impl>(api, connection, query, reuse_statement) }
{
}

statement::~statement() noexcept = default;

statement::statement(statement&&) = default;

statement& statement::operator=(statement&&) = default;

void statement::execute(const std::map<std::string, parameter>& parameters, const std::vector<result>& results)
{
	this->pimpl_->execute(parameters, results);
}

void statement::execute(const std::map<std::string, parameter>& parameters, const std::map<std::string, result>& results)
{
	this->pimpl_->execute(parameters, results);
}

bool statement::fetch()
{
	return this->pimpl_->fetch();
}

std::size_t statement::field_count()
{
	return this->pimpl_->field_count();
}

std::string statement::field_name(std::size_t index)
{
	return this->pimpl_->field_name(index);
}

std::uint64_t statement::affected_rows()
{
	return this->pimpl_->affected_rows();
}

/*static*/ void statement::execute(ipq_api* api, PGconn& connection, const std::string& query)
{
	std::shared_ptr<PGresult> result{ api->exec(&connection, query.c_str()), [api](PGresult* res) { api->clear(res); } };
	if (result)
	{
		const auto status = api->resultStatus(result.get());
		if (PGRES_TUPLES_OK != status && PGRES_COMMAND_OK != status)
		{
			ZOO_THROW_EXCEPTION(error{ api, "PQexec failed", connection, *result.get() });
		}
	}
	else
	{
		ZOO_THROW_EXCEPTION(error{ api, "PQexec failed", connection });
	}
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
