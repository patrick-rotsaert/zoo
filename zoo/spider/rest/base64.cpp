#include "zoo/spider/rest/base64.h"

#include <boost/beast/core/detail/base64.hpp>

namespace zoo {
namespace spider {

std::optional<std::string> base64::decode_to_string(std::string_view in)
{
	namespace b64 = boost::beast::detail::base64;

	const auto  dest_size = b64::decoded_size(in.length());
	std::string dest{};
	dest.resize(dest_size);
	const auto [bytes_written, bytes_read] = b64::decode(dest.data(), in.data(), in.length());
	if (bytes_read < in.length())
	{
		if (in.at(bytes_read) != '=')
		{
			return std::nullopt;
		}
	}
	if (bytes_written != dest_size)
	{
		dest.resize(bytes_written);
	}
	return dest;
}

std::string base64::encode(std::string_view in)
{
	namespace b64 = boost::beast::detail::base64;

	const auto  dest_size = b64::encoded_size(in.length());
	std::string dest{};
	dest.resize(dest_size);
	const auto bytes_written = b64::encode(dest.data(), in.data(), in.length());
	if (bytes_written != dest_size)
	{
		dest.resize(bytes_written);
	}
	return dest;
}

} // namespace spider
} // namespace zoo
