//
// Copyright (C) 2022-2026 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <gtest/gtest.h>

#include "zoo/spider/messages/file_response.h"
#include "zoo/spider/messages/message.h"

#include <boost/beast/http/verb.hpp>
#include <boost/filesystem.hpp>

#include <string_view>
#include <variant>

#if !defined(_WIN32)
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace zoo {
namespace spider {
namespace {

http::status status_of(const response_wrapper& rw)
{
	return std::visit([](const auto& r) { return r.result(); }, rw.value());
}

request make_get(std::string_view target)
{
	return request{ http::verb::get, target, 11 };
}

} // namespace

// A missing file must map to 404.
TEST(FileResponseErrors, MissingFileMapsToNotFound)
{
	namespace bfs  = boost::filesystem;
	const auto dir = bfs::temp_directory_path() / bfs::unique_path("spider-file-%%%%-%%%%");
	bfs::create_directories(dir);

	const auto rw = file_response::create(make_get("/files/nope.txt"), dir, "nope.txt");
	const auto st = status_of(rw);

	bfs::remove_all(dir);

	EXPECT_EQ(st, http::status::not_found) << "got " << static_cast<int>(st);
}

// A file that exists but cannot be opened due to permissions must map to 403 Forbidden, not 500
// Internal Server Error (which is both the wrong status and discloses server-side detail).
//
// POSIX-only: this relies on chmod() to make the file unreadable, and on not running as root (root
// bypasses permission checks). Windows uses ACLs and has no equivalent portable mechanism here.
#if !defined(_WIN32)

TEST(FileResponseErrors, PermissionDeniedMapsToForbidden)
{
	if (::geteuid() == 0)
	{
		GTEST_SKIP() << "running as root bypasses file permission checks";
	}

	namespace bfs  = boost::filesystem;
	const auto dir = bfs::temp_directory_path() / bfs::unique_path("spider-file-%%%%-%%%%");
	bfs::create_directories(dir);
	const auto file = dir / "secret.txt";
	{
		std::ofstream ofs{ file.string() };
		ofs << "top secret";
	}
	::chmod(file.string().c_str(), 0000);

	const auto rw = file_response::create(make_get("/files/secret.txt"), dir, "secret.txt");
	const auto st = status_of(rw);

	::chmod(file.string().c_str(), 0600);
	bfs::remove_all(dir);

	EXPECT_EQ(st, http::status::forbidden) << "got " << static_cast<int>(st);
}

#endif // !_WIN32

} // namespace spider
} // namespace zoo
