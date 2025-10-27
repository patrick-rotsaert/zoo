//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/detail/asyncbackend.h"
#include "zoo/squid/postgresql/detail/query.h"
#include "zoo/squid/postgresql/detail/queryparameters.h"
#include "zoo/squid/postgresql/detail/statementname.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/system/error_code.hpp>

#include <map>

namespace zoo {
namespace squid {
namespace postgresql {
namespace {

std::optional<std::string> pq_error_message(ipq_api& api, const PGconn& connection)
{
	const auto msg = api.errorMessage(&connection);
	if (msg)
	{
		return std::string{ msg };
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<std::string> pq_error_message(ipq_api& api, const PGresult& result, const PGconn& connection)
{
	const auto msg = api.resultErrorMessage(&result);
	if (msg)
	{
		return std::string{ msg };
	}
	else
	{
		return pq_error_message(api, connection);
	}
}

auto make_error_result(const std::error_code& ec)
{
	return async_error{ .ec = ec, .message = std::nullopt, .func = std::nullopt };
}

auto make_error_result(const boost::system::error_code& ec)
{
	return async_error{ .ec = std::make_error_code(static_cast<std::errc>(ec.value())), .message = std::nullopt, .func = "async_wait" };
}

auto make_error_result(ipq_api& api, std::string_view func, const PGconn& connection)
{
	return async_error{ .ec = std::nullopt, .message = pq_error_message(api, connection), .func = func };
}

async_exec_result make_exec_result(ipq_api& api, std::string_view func, std::shared_ptr<PGresult> result, const PGconn& connection)
{
	const auto status = api.resultStatus(result.get());

	if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
	{
		return async_error{ .ec = std::nullopt, .message = pq_error_message(api, *result, connection), .func = func };
	}

	return resultset{ &api, std::move(result) };
}

template<class Derived, class CompletionHandler>
class async_operation : public std::enable_shared_from_this<Derived>
{
	std::shared_ptr<async_operation> self()
	{
		return std::static_pointer_cast<Derived>(this->shared_from_this());
	}

public:
	virtual ~async_operation()
	{
		// stream_ takes ownership of the descriptor.
		// We need to prevent it being closed, because the PGconn object in fact owns it.
		if (this->stream_.is_open())
		{
			this->stream_.release();
		}
	}

protected:
	ipq_api*                            api_;
	std::shared_ptr<backend_connection> connection_;
	boost::asio::io_context&            io_;
	CompletionHandler                   handler_;

	boost::asio::posix::stream_descriptor stream_;
	boost::asio::io_context::strand       strand_;

	bool flushed_      = false;
	bool waiting_read  = false;
	bool waiting_write = false;

	explicit async_operation(ipq_api*                            api,
	                         std::shared_ptr<backend_connection> connection,
	                         boost::asio::io_context&            io,
	                         CompletionHandler                   handler)
	    : api_{ api }
	    , connection_{ std::move(connection) }
	    , io_{ io }
	    , handler_{ std::move(handler) }
	    , stream_{ io }
	    , strand_{ io }
	{
	}

	PGconn* setup_connection()
	{
		auto conn = this->connection_->native_connection().get();

		if (this->api_->setnonblocking(conn, 1))
		{
			this->fail("PQsetnonblocking", *conn);
			return nullptr;
		}

		const auto sock = this->api_->socket(conn);
		if (sock < 0)
		{
			this->fail("PQsocket", *conn);
			return nullptr;
		}

		this->stream_.assign(sock);

		return conn;
	}

	virtual void handle_result(std::shared_ptr<PGresult> result) = 0;

	void fail(std::string_view func, const PGconn& connection)
	{
		this->handler_(make_error_result(*this->api_, std::move(func), connection));
	}

	void fail(const std::error_code& ec)
	{
		this->handler_(make_error_result(ec));
	}

	void fail(const boost::system::error_code& ec)
	{
		this->handler_(make_error_result(ec));
	}

	void on_read_ready(const boost::system::error_code& ec)
	{
		this->waiting_read = false;

		if (ec)
		{
			return this->fail(ec);
		}

		auto conn = this->connection_->native_connection().get();

		if (this->api_->consumeInput(conn) != 1)
		{
			return this->fail("PQconsumeInput", *conn);
		}

		if (!this->flushed_)
		{
			return this->flush();
		}

		for (;;)
		{
			if (this->api_->isBusy(conn))
			{
				return this->wait_read();
			}

			std::shared_ptr<PGresult> result{ this->api_->getResult(conn), [this](PGresult* res) { this->api_->clear(res); } };
			if (result)
			{
				this->handle_result(std::move(result));
			}
			else
			{
				break;
			}
		}
	}

	void on_write_ready(const boost::system::error_code& ec)
	{
		this->waiting_write = false;

		if (ec)
		{
			return this->fail(ec);
		}

		if (!this->flushed_)
		{
			return this->flush();
		}
	}

	void wait_read()
	{
		if (!this->waiting_read)
		{
			this->stream_.async_wait(boost::asio::posix::stream_descriptor::wait_read,
			                         this->strand_.wrap(std::bind(&async_operation::on_read_ready, this->self(), std::placeholders::_1)));
			this->waiting_read = true;
		}
	}

	void wait_write()
	{
		if (!this->waiting_write)
		{
			this->stream_.async_wait(boost::asio::posix::stream_descriptor::wait_write,
			                         this->strand_.wrap(std::bind(&async_operation::on_write_ready, this->self(), std::placeholders::_1)));
			this->waiting_write = true;
		}
	}

	void flush()
	{
		const auto rc = this->api_->flush(this->connection_->native_connection().get());

		if (rc == 0)
		{
			this->flushed_ = true;
			this->wait_read();
		}
		else if (rc == 1)
		{
			this->wait_read();
			this->wait_write();
		}
		else
		{
			return this->fail("PQflush", *this->connection_->native_connection());
		}
	}
};

class async_exec_operation final : public async_operation<async_exec_operation, async_exec_completion_handler>
{
public:
	explicit async_exec_operation(ipq_api*                            api,
	                              std::shared_ptr<backend_connection> connection,
	                              boost::asio::io_context&            io,
	                              async_exec_completion_handler       handler)
	    : async_operation{ api, std::move(connection), io, std::move(handler) }
	{
	}

	void handle_result(std::shared_ptr<PGresult> result) override
	{
		this->handler_(make_exec_result(*this->api_, "PQsendQuery", std::move(result), *this->connection_->native_connection()));
	}

	void run(std::string_view query, std::initializer_list<std::pair<std::string_view, parameter_by_value>> params)
	{
		postgresql_query query_{ std::move(query) };

		std::map<std::string, parameter> params_{};
		for (auto&& pair : params)
		{
			params_.insert_or_assign(std::string{ pair.first }, std::move(pair.second));
		}
		query_parameters query_params{ query_, params_ };

		assert(query_params.parameter_count() == query_.parameter_count());

		if (auto conn = this->setup_connection())
		{
			if (this->api_->sendQueryParams(conn,
			                                query_.query().c_str(),
			                                query_params.parameter_count(),
			                                nullptr,
			                                query_params.parameter_values(),
			                                nullptr,
			                                nullptr,
			                                0) != 1)
			{
				return this->fail("PQsendQuery", *conn);
			}

			return this->flush();
		}
	}
};

class async_prepare_operation final : public async_operation<async_prepare_operation, async_prepare_completion_handler>
{
	std::string stmt_name_;

public:
	explicit async_prepare_operation(ipq_api*                            api,
	                                 std::shared_ptr<backend_connection> connection,
	                                 boost::asio::io_context&            io,
	                                 async_prepare_completion_handler    handler)
	    : async_operation{ api, std::move(connection), io, std::move(handler) }
	    , stmt_name_{ next_statement_name() }
	{
	}

	void handle_result(std::shared_ptr<PGresult> result) override
	{
		const auto status = this->api_->resultStatus(result.get());

		if (status == PGRES_COMMAND_OK)
		{
			return this->handler_(async_prepared_statement{ this->api_, this->connection_, this->io_, this->stmt_name_ });
		}
		else
		{
			return this->handler_(async_error{ .ec      = std::nullopt,
			                                   .message = pq_error_message(*this->api_, *result, *this->connection_->native_connection()),
			                                   .func    = "PQsendPrepare" });
		}
	}

	void run(std::string_view query)
	{
		postgresql_query query_{ std::move(query) };

		if (auto conn = this->setup_connection())
		{
			if (this->api_->sendPrepare(conn, this->stmt_name_.c_str(), query_.query().c_str(), query_.parameter_count(), nullptr) != 1)
			{
				return this->fail("PQsendPrepare", *conn);
			}

			return this->flush();
		}
	}
};

class async_exec_prepared_operation final : public async_operation<async_exec_prepared_operation, async_exec_completion_handler>
{
public:
	explicit async_exec_prepared_operation(ipq_api*                            api,
	                                       std::shared_ptr<backend_connection> connection,
	                                       boost::asio::io_context&            io,
	                                       async_exec_completion_handler       handler)
	    : async_operation{ api, std::move(connection), io, std::move(handler) }
	{
	}

	void handle_result(std::shared_ptr<PGresult> result) override
	{
		this->handler_(make_exec_result(*this->api_, "sendQueryPrepared", std::move(result), *this->connection_->native_connection()));
	}

	void run(std::string_view stmt_name, std::initializer_list<std::pair<std::string_view, parameter_by_value>> params)
	{
		//@@ TODO
		(void)stmt_name;
		(void)params;

		return this->handler_(async_error{ .ec = std::nullopt, .message = "*NOT IMPLEMENTED*", .func = std::nullopt });

		// std::map<std::string, parameter> params_{};
		// for (auto&& pair : params)
		// {
		// 	params_.insert_or_assign(std::string{ pair.first }, std::move(pair.second));
		// }
		// query_parameters query_params{ query_, params_ };

		// assert(query_params.parameter_count() == query_.parameter_count());

		// if (auto conn = this->setup_connection())
		// {
		// 	if (this->api_->sendQueryPrepared(conn,
		// 	                                query_.query().c_str(),
		// 	                                query_params.parameter_count(),
		// 	                                nullptr,
		// 	                                query_params.parameter_values(),
		// 	                                nullptr,
		// 	                                nullptr,
		// 	                                0) != 1)
		// 	{
		// 		return this->fail("sendQueryPrepared", *conn);
		// 	}

		// 	return this->flush();
		// }
	}
};

} // namespace

void async_backend::exec(ipq_api*                                                               api,
                         std::shared_ptr<backend_connection>                                    connection,
                         boost::asio::io_context&                                               io,
                         std::string_view                                                       query,
                         std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
                         async_exec_completion_handler                                          handler)
{
	return std::make_shared<async_exec_operation>(api, std::move(connection), io, std::move(handler))
	    ->run(std::move(query), std::move(params));
}

void async_backend::prepare(ipq_api*                            api,
                            std::shared_ptr<backend_connection> connection,
                            boost::asio::io_context&            io,
                            std::string_view                    query,
                            async_prepare_completion_handler    handler)
{
	return std::make_shared<async_prepare_operation>(api, std::move(connection), io, std::move(handler))->run(std::move(query));
}

void async_backend::exec_prepared(ipq_api*                                                               api,
                                  std::shared_ptr<backend_connection>                                    connection,
                                  boost::asio::io_context&                                               io,
                                  std::string_view                                                       stmt_name,
                                  std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
                                  async_exec_completion_handler                                          handler)
{
	return std::make_shared<async_exec_prepared_operation>(api, std::move(connection), io, std::move(handler))
	    ->run(std::move(stmt_name), std::move(params));
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
