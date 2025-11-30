#include "auth.h"

#include "zoo/spider/rest/base64.h"
#include "zoo/common/conversion/conversion.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <fmt/format.h>

#include <sstream>

namespace demo {
namespace {

const auto flags = boost::archive::no_header | boost::archive::no_tracking;

// using iarchive = boost::archive::binary_iarchive;
// using oarchive = boost::archive::binary_oarchive;

using iarchive = boost::archive::text_iarchive;
using oarchive = boost::archive::text_oarchive;

} // namespace

template<typename Archive>
void serialize(Archive& ar, BearerAuthData& a, const unsigned int)
{
	ar & a.userName & a.exp;
}

// Totally insecure way of token verification ;-)
// This is just for demonstration purposes.
// To verify a JWT token, jwt-cpp could be used.

std::expected<auth_data, auth_error> BearerAuthData::verify(const bearer_authorization& ba, std::string_view token)
{
	const auto decoded = base64::decode_to_string(token);
	if (!decoded)
	{
		const auto message = fmt::format("{}: Invalid base64 data", ba.scheme_name());
		return std::unexpected(ba.make_verification_error(message, { { "error", "invalid_token" }, { "error_description", message } }));
	}
	std::istringstream ss{};
	ss.str(decoded.value());
	iarchive ia{ ss, flags };
	auto     data = std::make_unique<BearerAuthData>();
	try
	{
		ia >> *data;
	}
	catch (const std::exception& e)
	{
		const auto message = fmt::format("{}: Invalid token encoding: {}", ba.scheme_name(), e.what());
		return std::unexpected(ba.make_verification_error(message, { { "error", "invalid_token" }, { "error_description", message } }));
	}
	if (data->exp < boost::posix_time::second_clock::universal_time())
	{
		const auto message = fmt::format("{}: Token expired on {}", ba.scheme_name(), zoo::conversion::boost_ptime_to_iso8601(data->exp));
		return std::unexpected(ba.make_verification_error(message, { { "error", "token_expired" }, { "error_description", message } }));
	}
	return std::move(data);
}

std::string BearerAuthData::asToken() const
{
	std::ostringstream ss{};
	oarchive           oa{ ss, flags };
	oa << *this;
	return base64::encode(ss.str());
}

} // namespace demo
