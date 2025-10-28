#include "zoo/squid/postgresql/asyncpreparedstatement.h"
#include "zoo/squid/postgresql/backendconnection.h"
#include "zoo/squid/postgresql/detail/query.h"
#include "zoo/common/logging/logging.h"

namespace zoo {
namespace squid {
namespace postgresql {

async_prepared_statement::async_prepared_statement(ipq_api*                            api,
                                                   std::shared_ptr<backend_connection> connection,
                                                   boost::asio::io_context&            io,
                                                   std::unique_ptr<postgresql_query>   query,
                                                   std::string                         stmt_name)
    : api_{ api }
    , connection_{ std::move(connection) }
    , io_{ &io }
    , query_{ std::move(query) }
    , stmt_name_{ std::move(stmt_name) }
{
}

async_prepared_statement::~async_prepared_statement()
{
	try
	{
		if (this->connection_)
		{
			this->connection_->run_async_exec(
			    *this->io_, "DEALLOCATE " + this->stmt_name_, {}, [stmt_name = this->stmt_name_](auto result) {
				    if (std::holds_alternative<postgresql::async_error>(result))
				    {
					    const auto& err = std::get<postgresql::async_error>(result);
					    ZOO_LOG(warn, "Deallocate failed: {}", err.format());
				    }
			    });
		}
	}
	catch (const std::exception& e)
	{
		ZOO_LOG(warn, "{}", e.what());
	}
}

async_prepared_statement::async_prepared_statement(async_prepared_statement&& src)        = default;
async_prepared_statement& async_prepared_statement::operator=(async_prepared_statement&&) = default;

void async_prepared_statement::async_exec(std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
                                          async_exec_completion_handler                                          handler)
{
	this->connection_->run_async_exec_prepared(*this->io_, *this->query_, this->stmt_name_, std::move(params), std::move(handler));
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
