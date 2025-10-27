#include "zoo/squid/postgresql/detail/statementname.h"

#include <fmt/format.h>

#include <atomic>
#include <cstdint>

namespace zoo {
namespace squid {
namespace postgresql {

std::string next_statement_name()
{
	static std::atomic<uint64_t> statement_number = 0;
	return fmt::format("s_{}", ++statement_number);
}

} // namespace postgresql
} // namespace squid
} // namespace zoo
