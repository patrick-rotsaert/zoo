//
// Copyright (C) 2022-2025 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/squid/postgresql/detail/asyncbackendstatement.h"
#include "zoo/squid/postgresql/detail/query.h"
#include "zoo/squid/postgresql/detail/queryparameters.h"

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

async_result make_error_result(const std::error_code& ec)
{
	return async_error{ .ec = ec, .message = std::nullopt, .func = std::nullopt };
}

async_result make_error_result(const boost::system::error_code& ec)
{
	return async_error{ .ec = std::make_error_code(static_cast<std::errc>(ec.value())), .message = std::nullopt, .func = "async_wait" };
}

async_result make_error_result(ipq_api& api, std::string_view func, const PGconn& connection)
{
	return async_error{ .ec = std::nullopt, .message = pq_error_message(api, connection), .func = func };
}

async_result make_result(ipq_api& api, std::shared_ptr<PGresult> result, const PGconn& connection)
{
	const auto status = api.resultStatus(result.get());

	if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
	{
		return async_error{ .ec = std::nullopt, .message = pq_error_message(api, *result, connection), .func = std::nullopt };
	}

	return resultset{ &api, std::move(result) };
}

class async_backend_statement_impl final : public std::enable_shared_from_this<async_backend_statement_impl>
{
	ipq_api*                 api_;
	std::shared_ptr<PGconn>  connection_;
	async_completion_handler handler_;

	boost::asio::posix::stream_descriptor stream_;
	boost::asio::io_context::strand       strand_;

	bool flushed_      = false;
	bool waiting_read  = false;
	bool waiting_write = false;

public:
	explicit async_backend_statement_impl(ipq_api*                 api,
	                                      std::shared_ptr<PGconn>  connection,
	                                      boost::asio::io_context& io,
	                                      async_completion_handler handler)
	    : api_{ api }
	    , connection_{ std::move(connection) }

	    , handler_{ std::move(handler) }
	    , stream_{ io }
	    , strand_{ io }
	{
	}

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

	void handle_result(std::shared_ptr<PGresult> result)
	{
		this->handler_(make_result(*this->api_, std::move(result), *this->connection_));
	}

	void on_read_ready(const boost::system::error_code& ec)
	{
		this->waiting_read = false;

		if (ec)
		{
			return this->fail(ec);
		}

		auto conn = this->connection_.get();

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
			                         this->strand_.wrap(std::bind(
			                             &async_backend_statement_impl::on_read_ready, this->shared_from_this(), std::placeholders::_1)));
			this->waiting_read = true;
		}
	}

	void wait_write()
	{
		if (!this->waiting_write)
		{
			this->stream_.async_wait(boost::asio::posix::stream_descriptor::wait_write,
			                         this->strand_.wrap(std::bind(
			                             &async_backend_statement_impl::on_write_ready, this->shared_from_this(), std::placeholders::_1)));
			this->waiting_write = true;
		}
	}

	void flush()
	{
		const auto rc = this->api_->flush(this->connection_.get());

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
			return this->fail("PQflush", *this->connection_);
		}
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

		auto conn = this->connection_.get();

		if (this->api_->setnonblocking(conn, 1))
		{
			return this->fail("PQsetnonblocking", *conn);
		}

		const auto sock = this->api_->socket(conn);
		if (sock < 0)
		{
			return this->fail("PQsocket", *conn);
		}

		this->stream_.assign(sock);

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
};

} // namespace

void async_backend_statement::run(ipq_api*                                                               api,
                                  std::shared_ptr<PGconn>                                                connection,
                                  boost::asio::io_context&                                               io,
                                  std::string_view                                                       query,
                                  std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
                                  async_completion_handler                                               handler)
{
	return std::make_shared<async_backend_statement_impl>(api, std::move(connection), io, std::move(handler))
	    ->run(std::move(query), std::move(params));
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
