//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/issh_identity_factory.h"
#include "zoo/fs/sftp/config.h"
#include <gmock/gmock.h>

namespace zoo {
namespace fs {
namespace sftp {

class ZOO_FS_SFTP_API mock_ssh_identity_factory : public issh_identity_factory
{
public:
	explicit mock_ssh_identity_factory();
	~mock_ssh_identity_factory() override;

	MOCK_METHOD(std::vector<std::shared_ptr<ssh_identity>>, create, (), (override));
};

// This regex search and replace patterns can help to convert declarations into MOCK_METHOD macro calls
// Search pattern: ^(.+)\s([a-zA-Z_][a-zA-Z0-9_]*)\s*(\([^)]*\))\s*override\s*;\s*$
// Replace pattern: MOCK_METHOD(\1,\2,\3,(override));

using nice_mock_ssh_identity_factory   = testing::NiceMock<mock_ssh_identity_factory>;
using naggy_mock_ssh_identity_factory  = testing::NaggyMock<mock_ssh_identity_factory>;
using strict_mock_ssh_identity_factory = testing::StrictMock<mock_ssh_identity_factory>;

} // namespace sftp
} // namespace fs
} // namespace zoo
