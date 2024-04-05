//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/iaccess.h"
#include "zoo/fs/core/ifile.h"
#include "zoo/fs/core/attributes.h"
#include "zoo/fs/core/direntry.h"
#include <gmock/gmock.h>

namespace zoo {
namespace fs {

class ZOO_EXPORT mock_access : public iaccess
{
public:
	explicit mock_access();
	~mock_access() override;

	MOCK_METHOD(bool, is_remote, (), (const, override));
	MOCK_METHOD(std::vector<direntry>, ls, (const fspath& dir), (override));
	MOCK_METHOD(bool, exists, (const fspath& path), (override));
	MOCK_METHOD(std::optional<attributes>, try_stat, (const fspath& path), (override));
	MOCK_METHOD(attributes, stat, (const fspath& path), (override));
	MOCK_METHOD(std::optional<attributes>, try_lstat, (const fspath& path), (override));
	MOCK_METHOD(attributes, lstat, (const fspath& path), (override));
	MOCK_METHOD(void, remove, (const fspath& path), (override));
	MOCK_METHOD(void, mkdir, (const fspath& path, bool parents), (override));
	MOCK_METHOD(void, rename, (const fspath& oldpath, const fspath& newpath), (override));
	MOCK_METHOD(std::unique_ptr<ifile>, open, (const fspath& path, int flags, mode_t mode), (override));
	MOCK_METHOD(std::shared_ptr<iwatcher>, create_watcher, (const fspath& dir, int cancelfd), (override));
};

// This regex search and replace patterns can help to convert declarations into MOCK_METHOD macro calls
// Search pattern: ^(.+)\s([a-zA-Z_][a-zA-Z0-9_]*)\s*(\([^)]*\))\s*override\s*;\s*$
// Replace pattern: MOCK_METHOD(\1,\2,\3,(override));

using nice_mock_access   = testing::NiceMock<mock_access>;
using naggy_mock_access  = testing::NaggyMock<mock_access>;
using strict_mock_access = testing::StrictMock<mock_access>;

} // namespace fs
} // namespace zoo
