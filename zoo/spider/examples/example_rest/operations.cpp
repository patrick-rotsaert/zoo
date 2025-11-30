#include "operations.h"
#include "test_png_image.h"

#include "zoo/spider/exception.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/throw_exception.h"
#include "zoo/common/misc/formatters.hpp"
#include "zoo/common/misc/quoted_c.h"
#include "zoo/common/misc/rlws.hpp"

#include <fmt/format.h>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace demo {

using namespace std::string_view_literals;
namespace {

struct exception : public exception_base
{
};

} // namespace

std::vector<Customer> Operations::listCustomers()
{
	return { Customer{ 42, "The Customer Inc", { Status::available, Status::cancelled } } };
}

Customer Operations::getCustomer(std::uint64_t id, const std::optional<std::string>& serial, std::optional<Status>)
{
	zlog(debug, "get customer id={}, serial={}", id, serial);
	return Customer{ id, "The Customer Inc" };
}

Customer Operations::createCustomer(const Customer& c)
{
	zlog(debug, "create customer {}", c.id);
	return c;
}

void Operations::noop()
{
	zlog(debug, "noop");
}

status_result<status::not_implemented, Error> Operations::fail(std::optional<bool> useException)
{
	zlog(debug, "fail, use exception is {}", useException);

	if (useException.value_or(false))
	{
		ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "The exception message" } << exception::code{ 42 }
		                                << exception::status{ status::not_implemented });
	}
	else
	{
		// return make_status_result<status::not_implemented>(Error::create(42, "The error message"));
		return Error::create(42, "The error message");
	}
}

image_container Operations::image()
{
	return image_container::create_view("image/png"sv, test_png_image::data());
}

std::variant<status_result<status::ok, TestString>,
             status_result<status::ok, image_container>, // not used, for OAS demo only
             status_result<status::not_found, std::string>,
             status_result<status::not_found, Error> // not used, for OAS demo only
             >
Operations::test(bool found)
{
	if (found)
	{
		return make_status_result<status::ok>(TestString{ true, "hello" });
	}
	else
	{
		return make_status_result<status::not_found>(std::string{ "FAIL" });
	}
}

html_container Operations::testBasicAuth(const BasicAuthData& auth)
{
	zlog(info, "user name is {}", zoo::quoted_c(auth.userName));
	return html_container::create(std::string{ html_container::CONTENT_TYPE },
	                              fmt::format(R"(
		<html>
		 <body>
		  <h1>Hi, {}</h1>
		 </body>
		</html>
		)"_rlws,
	                                          auth.userName));
}

html_container Operations::testBearerAuth(const BearerAuthData& auth)
{
	zlog(info, "user name is {}", zoo::quoted_c(auth.userName));
	return html_container::create(std::string{ html_container::CONTENT_TYPE },
	                              fmt::format(R"(
		<html>
		 <body>
		  <h1>Hello, {}</h1>
		 </body>
		</html>
		)"_rlws,
	                                          auth.userName));
}

std::string Operations::login(const std::string& user)
{
	zlog(debug, "user = {}", zoo::quoted_c(user));
	BearerAuthData data{};
	data.userName = user;
	data.exp      = boost::posix_time::second_clock::universal_time() + boost::posix_time::minutes{ 1 };
	return data.asToken();
}

} // namespace demo
