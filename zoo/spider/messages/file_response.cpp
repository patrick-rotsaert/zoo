//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/messages/file_response.h"
#include "zoo/spider/messages/error_response.h"
#include "zoo/spider/messages/tracked_file.h"
#include "zoo/spider/messages/ifile_event_listener.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

#include <boost/beast/http/file_body.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace zoo {
namespace spider {

namespace {

beast::string_view mime_type(const boost::filesystem::path& path)
{
	const auto ext = path.extension().string();
	using beast::iequals;
	if (iequals(ext, ".htm"))
		return "text/html";
	if (iequals(ext, ".html"))
		return "text/html";
	if (iequals(ext, ".php"))
		return "text/html";
	if (iequals(ext, ".css"))
		return "text/css";
	if (iequals(ext, ".txt"))
		return "text/plain";
	if (iequals(ext, ".js"))
		return "application/javascript";
	if (iequals(ext, ".json"))
		return "application/json";
	if (iequals(ext, ".xml"))
		return "application/xml";
	if (iequals(ext, ".swf"))
		return "application/x-shockwave-flash";
	if (iequals(ext, ".flv"))
		return "video/x-flv";
	if (iequals(ext, ".png"))
		return "image/png";
	if (iequals(ext, ".jpe"))
		return "image/jpeg";
	if (iequals(ext, ".jpeg"))
		return "image/jpeg";
	if (iequals(ext, ".jpg"))
		return "image/jpeg";
	if (iequals(ext, ".gif"))
		return "image/gif";
	if (iequals(ext, ".bmp"))
		return "image/bmp";
	if (iequals(ext, ".ico"))
		return "image/vnd.microsoft.icon";
	if (iequals(ext, ".tiff"))
		return "image/tiff";
	if (iequals(ext, ".tif"))
		return "image/tiff";
	if (iequals(ext, ".svg"))
		return "image/svg+xml";
	if (iequals(ext, ".svgz"))
		return "image/svg+xml";
	return "application/octet-stream";
}

template<class FileBody, class CreateFile>
response_wrapper create_impl(const request& req, const boost::filesystem::path& doc_root, beast::string_view path, CreateFile&& create_file)
{
	const auto file_path = doc_root / std::string{ path };

	// Use the non-throwing overload: relative() calls weakly_canonical() which stat()s the path and
	// throws on e.g. an over-long request path (ENAMETOOLONG), a symlink loop or EACCES. A throw here
	// would escape into the Asio completion handler and terminate the server.
	auto       rel_ec   = beast::error_code{};
	const auto rel_path = boost::filesystem::relative(file_path, doc_root, rel_ec);
	if (rel_ec)
	{
		ZOO_LOG(err, "Cannot resolve {} relative to {}: {}", file_path, doc_root, rel_ec.message());
		return bad_request::create(req);
	}
	if (!rel_path.empty() && rel_path.begin()->filename_is_dot_dot())
	{
		ZOO_LOG(err, "{} is not a child path of {}", file_path, doc_root);
		return bad_request::create(req);
	}

	// A HEAD response carries no body, so do not open or track the file (opening a tracked file would
	// fire spurious open/close events, e.g. a bogus "incomplete download"). Report the size only.
	if (req.method() == verb::head)
	{
		auto       size_ec = beast::error_code{};
		const auto size    = boost::filesystem::file_size(file_path, size_ec);
		if (size_ec)
		{
			return not_found::create(req);
		}

		auto res = http::response<http::empty_body>{ http::status::ok, req.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, mime_type(file_path));
		res.content_length(size);
		res.keep_alive(req.keep_alive());
		return res;
	}

	auto file = create_file();

	auto ec = beast::error_code{};

	file.open(file_path.string().c_str(), beast::file_mode::scan, ec);

	if (ec)
	{
		ZOO_LOG(err, "{}: {}", file_path.string(), ec.message());
	}

	// The file does not exist, or the path resolves to a directory: 404.
	if (ec == beast::errc::no_such_file_or_directory || ec == beast::errc::is_a_directory)
	{
		return not_found::create(req);
	}

	// The file exists but cannot be opened due to permissions: 403 (not 500, which is both the wrong
	// status and discloses server-side detail).
	if (ec == beast::errc::permission_denied)
	{
		return error_response<status::forbidden>::create(req);
	}

	// Handle an unknown error
	if (ec)
	{
		return internal_server_error::create(req);
	}

	auto body = typename FileBody::value_type();
	body.reset(std::move(file), ec);

	const auto size = body.size();

	auto res =
	    http::response<FileBody>{ std::piecewise_construct, std::make_tuple(std::move(body)), std::make_tuple(status::ok, req.version()) };
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, mime_type(file_path));
	res.content_length(size);
	res.keep_alive(req.keep_alive());
	return res;
}

template<class FileBody, class CreateFile>
response_wrapper create_impl(const boost::filesystem::path& doc_root, beast::string_view path, CreateFile&& create_file)
{
	const auto file_path = doc_root / std::string{ path };

	// Use the non-throwing overload (see the other create_impl for the rationale).
	auto       rel_ec   = beast::error_code{};
	const auto rel_path = boost::filesystem::relative(file_path, doc_root, rel_ec);
	if (rel_ec)
	{
		ZOO_LOG(err, "Cannot resolve {} relative to {}: {}", file_path, doc_root, rel_ec.message());
		return bad_request::create();
	}
	if (!rel_path.empty() && rel_path.begin()->filename_is_dot_dot())
	{
		ZOO_LOG(err, "{} is not a child path of {}", file_path, doc_root);
		return bad_request::create();
	}

	auto file = create_file();

	auto ec = beast::error_code{};

	file.open(file_path.string().c_str(), beast::file_mode::scan, ec);

	if (ec)
	{
		ZOO_LOG(err, "{}: {}", file_path.string(), ec.message());
	}

	// The file does not exist, or the path resolves to a directory: 404.
	if (ec == beast::errc::no_such_file_or_directory || ec == beast::errc::is_a_directory)
	{
		return not_found::create();
	}

	// The file exists but cannot be opened due to permissions: 403.
	if (ec == beast::errc::permission_denied)
	{
		return error_response<status::forbidden>::create();
	}

	// Handle an unknown error
	if (ec)
	{
		return internal_server_error::create();
	}

	auto body = typename FileBody::value_type();
	body.reset(std::move(file), ec);

	const auto size = body.size();

	auto res = http::response<FileBody>{ std::piecewise_construct, std::make_tuple(std::move(body)) };
	res.result(status::ok);
	res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(http::field::content_type, mime_type(file_path));
	res.content_length(size);
	return res;
}

} // namespace

response_wrapper file_response::create(const request&                          req,
                                       const boost::filesystem::path&          doc_root,
                                       string_view                             path,
                                       std::unique_ptr<ifile_event_listener>&& event_listener)
{
	using tracked_file_body = http::basic_file_body<tracked_file>;
	return create_impl<tracked_file_body>(req, doc_root, path, [&]() { return tracked_file_body::file_type{ std::move(event_listener) }; });
}

response_wrapper file_response::create(const request& req, const fs::path& doc_root, string_view path)
{
	return create_impl<http::file_body>(req, doc_root, path, []() { return http::file_body::file_type{}; });
}

response_wrapper file_response::create(const fs::path& doc_root, string_view path, std::unique_ptr<ifile_event_listener>&& event_listener)
{
	using tracked_file_body = http::basic_file_body<tracked_file>;
	return create_impl<tracked_file_body>(doc_root, path, [&]() { return tracked_file_body::file_type{ std::move(event_listener) }; });
}

response_wrapper file_response::create(const fs::path& doc_root, string_view path)
{
	return create_impl<http::file_body>(doc_root, path, []() { return http::file_body::file_type{}; });
}

} // namespace spider
} // namespace zoo
