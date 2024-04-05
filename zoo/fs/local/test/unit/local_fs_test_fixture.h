//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/fspath.h"
#include <boost/filesystem/file_status.hpp>
#include <gtest/gtest.h>
#include <optional>

namespace zoo {
namespace fs {
namespace local {

class LocalFsTestFixture : public testing::Test
{
	mutable std::optional<fspath> work_dir_;

protected:
	LocalFsTestFixture();

	void SetUp() override;
	void TearDown() override;

	const fspath&                  work_dir() const;
	void                           touch(const fspath& p) const;
	boost::filesystem::file_status stat(const fspath& p) const;
	boost::filesystem::file_status lstat(const fspath& p) const;
};

} // namespace local
} // namespace fs
} // namespace zoo
