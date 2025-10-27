#include "zoo/squid/postgresql/asyncpreparedstatement.h"
#include "zoo/squid/postgresql/backendconnection.h"

namespace zoo {
namespace squid {
namespace postgresql {

async_prepared_statement::async_prepared_statement(ipq_api*                            api,
                                                   std::shared_ptr<backend_connection> connection,
                                                   boost::asio::io_context&            io,
                                                   std::string                         stmt_name)
    : api_{ api }
    , connection_{ std::move(connection) }
    , io_{ &io }
    , stmt_name_{ std::move(stmt_name) }
{
}

async_prepared_statement::~async_prepared_statement()
{
	//@@TODO: deallocate stmt_name
}

async_prepared_statement::async_prepared_statement(async_prepared_statement&& src)        = default;
async_prepared_statement& async_prepared_statement::operator=(async_prepared_statement&&) = default;

void async_prepared_statement::async_exec(std::initializer_list<std::pair<std::string_view, parameter_by_value>> params,
                                          async_exec_completion_handler                                          handler)
{
	this->connection_->run_async_exec_prepared(*this->io_, this->stmt_name_, std::move(params), std::move(handler));
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
