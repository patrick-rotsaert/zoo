#include "controller.h"
#include "operations.h"

#include "zoo/spider/rest/apikeyauthorization.h"
#include "zoo/spider/rest/basicauthorization.h"
#include "zoo/spider/rest/bearerauthorization.h"
#include "zoo/spider/json_util.h"

namespace demo {

using namespace zoo::spider;

Controller::Controller()
    : rest_controller{ openapi_settings{ .strip_ns = "demo::", .info_title = "Demo API", .info_version = "1.0" } }
{
	auto apiKeyAuth = std::make_shared<api_key_authorization>("ApiKeyAuth", api_key_authorization::source::header, "X-Api-Key", "123456");

	auto basicAuth = std::make_shared<basic_authorization>(
	    "BasicAuth",
	    [](std::string_view u, std::string_view p) -> std::expected<auth_data, std::string> {
		    if (u == "me" && p == "pass")
		    {
			    auto auth      = std::make_unique<BasicAuthData>();
			    auth->userName = u;
			    return auth;
		    }
		    else
		    {
			    return std::unexpected(std::string{ "Invalid user name / password" });
		    }
	    },
	    "Demo");

	auto bearerAuth = std::make_shared<bearer_authorization>("BearerAuth", &BearerAuthData::verify, "Demo");

	set_global_security({ {
	    { apiKeyAuth, {} },
	} });

	using p = rest_controller::p;

	add_operation(rest_operation{ .method       = verb::get,
	                              .path         = path_spec{ "api/v1" },
	                              .operation_id = "getOpenApi",
	                              .summary      = "Get the Open API specification.",
	                              .sec          = security{} },
	              &Controller::getOpenApi,
	              p::request{});

	add_operation(rest_operation{ .method       = verb::get,
	                              .path         = path_spec{ "api" } / "v1" / "customers",
	                              .operation_id = "listCustomers",
	                              .summary      = "Get a list of customers" },
	              &Operations::listCustomers);

	add_operation(rest_operation{ .method       = verb::get,
	                              .path         = path_spec{ "api" } / "v1" / "customers" / "{id}",
	                              .operation_id = "getCustomer",
	                              .summary      = "Get a single customer" },
	              &Operations::getCustomer,
	              p::path{ "id" },
	              p::query{ "serial" },
	              p::query{ "status" });

	add_operation(rest_operation{ .method       = verb::post,
	                              .path         = path_spec{ "api" } / "v1" / "customers",
	                              .operation_id = "createCustomer",
	                              .summary      = "Create a customer" },
	              &Operations::postCustomer,
	              p::json{});

	add_operation(rest_operation{ .method       = verb::get,
	                              .path         = path_spec{ "api" } / "v1" / "noop",
	                              .operation_id = "noop",
	                              .summary      = "An operation that accepts nothing, does nothing and returns nothing" },
	              &Operations::noop);

	add_operation(rest_operation{ .method       = verb::get,
	                              .path         = path_spec{ "api" } / "v1" / "fail",
	                              .operation_id = "fail",
	                              .summary      = "Operation that always fails" },
	              &Operations::fail,
	              p::query{ "exception", "If true then the operation fails with an exception. Default false." });

	add_operation(
	    rest_operation{
	        .method = verb::get, .path = path_spec{ "api" } / "v1" / "image", .operation_id = "getImage", .summary = "Get the image" },
	    &Operations::image);

	add_operation(
	    rest_operation{ .method = verb::get, .path = path_spec{ "api" } / "v1" / "test", .operation_id = "test", .summary = "Just a test" },
	    &Operations::test,
	    p::query{ "found" });

	add_operation(rest_operation{ .method       = verb::get,
	                              .path         = path_spec{ "api" } / "v1" / "basic",
	                              .operation_id = "testBasicAuth",
	                              .summary      = "Test basic authentication",
	                              .sec          = security{ { { basicAuth, {} }, { apiKeyAuth, {} } } } },
	              &Operations::testBasicAuth,
	              p::auth{ basicAuth->scheme_name() });

	add_operation(rest_operation{ .method       = verb::get,
	                              .path         = path_spec{ "api" } / "v1" / "bearer",
	                              .operation_id = "testBearerAuth",
	                              .summary      = "Test bearer authentication\n"
	                                              "Use the login endpoint to get a token",
	                              .sec          = security{ { { bearerAuth, {} }, { apiKeyAuth, {} } } } },
	              &Operations::testBearerAuth,
	              p::auth{ bearerAuth->scheme_name() });

	add_operation(
	    rest_operation{
	        .method = verb::get, .path = path_spec{ "api" } / "v1" / "login", .operation_id = "login", .summary = "Get a bearer token" },
	    &Operations::login,
	    p::query{ "userName" });
}

std::string Controller::openApiSpec() const
{
	const auto& jv = oas().spec();
	// return boost::json::serialize(jv);
	return json_util::pretty_print(jv);
}

response Controller::getOpenApi(const request& req)
{
	return json_response::create(req, status::ok, oas().spec());
}

} // namespace demo
