#include "operations.h"
#include "test_png_image.h"

#include "zoo/spider/exception.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/throw_exception.h"
#include "zoo/common/misc/formatters.hpp"

namespace demo {

using namespace std::string_view_literals;
namespace {

struct exception : public exception_base
{
};

} // namespace

std::vector<Customer> Operations::listCustomers(std::string api_key)
{
	if (api_key != "the_api_key")
	{
		ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "Bad API key" } << exception::status{ status::unauthorized });
	}
	return { Customer{ 42, "The Customer Inc", { Status::available, Status::cancelled } } };
}

Customer Operations::getCustomer(std::uint64_t                       id,
                                 const std::optional<std::string>&   serial,
                                 const boost::optional<std::string>& api_key,
                                 std::optional<Status>)
{
	zlog(debug, "get customer id={}, serial={}, api_key={}", id, serial, api_key);
	if (!api_key)
	{
		ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "An API key is required" } << exception::status{ status::unauthorized });
	}
	else if (api_key.value() != "the_api_key")
	{
		ZOO_THROW_EXCEPTION(exception{} << exception::mesg{ "Bad API key" } << exception::status{ status::unauthorized });
	}
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
		return make_status_result<status::not_implemented>(Error::create(42, "The error message"));
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

} // namespace demo
