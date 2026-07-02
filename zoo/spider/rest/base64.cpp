#include "zoo/spider/rest/base64.h"

#include <boost/beast/core/detail/base64.hpp>

namespace zoo {
namespace spider {

std::optional<std::string> base64::decode_to_string(std::string_view in)
{
	namespace b64 = boost::beast::detail::base64;

	// beast::detail::base64::decoded_size(n) == n / 4 * 3 and assumes padded input (n % 4 == 0).
	// For attacker-supplied, possibly-unpadded input, beast::decode writes up to 2 bytes beyond
	// that (it emits the partial trailing group), which would overflow the buffer. Allocate the
	// worst-case size and shrink to the number of bytes actually written.
	const auto  capacity = ((in.length() + 3u) / 4u) * 3u;
	std::string dest{};
	dest.resize(capacity);
	const auto [bytes_written, bytes_read] = b64::decode(dest.data(), in.data(), in.length());
	if (bytes_read < in.length())
	{
		if (in.at(bytes_read) != '=')
		{
			return std::nullopt;
		}
	}
	dest.resize(bytes_written);
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
