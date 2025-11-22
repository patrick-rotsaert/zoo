#include "zoo/spider/binary_response.h"

#include <boost/beast/version.hpp>

namespace zoo {
namespace spider {

binary_response::response
binary_response::create(const request& req, http::status status, std::string_view content_type, byte_string_view body)
{
	auto res = response{ status, req.version() };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, content_type);
	res.keep_alive(req.keep_alive());
	res.body().assign(body.begin(), body.end());
	res.prepare_payload();
	return res;
}

binary_response::response binary_response::create(http::status status, std::string_view content_type, byte_string_view body)
{
	auto res = response{};
	res.result(status);
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, content_type);
	res.body().assign(body.begin(), body.end());
	res.prepare_payload();
	return res;
}

binary_response::response
binary_response::create(const request& req, http::status status, std::string_view content_type, std::vector<std::uint8_t> body)
{
	auto res = response{ status, req.version() };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, content_type);
	res.keep_alive(req.keep_alive());
	res.body() = std::move(body);
	res.prepare_payload();
	return res;
}

binary_response::response binary_response::create(http::status status, std::string_view content_type, std::vector<std::uint8_t> body)
{
	auto res = response{};
	res.result(status);
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, content_type);
	res.body() = std::move(body);
	res.prepare_payload();
	return res;
}

} // namespace spider
} // namespace zoo
