//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/issh_knownhosts.h"
#include "zoo/common/api.h"
#include <gmock/gmock.h>

namespace zoo {
namespace fs {
namespace sftp {

class ZOO_EXPORT mock_ssh_known_hosts : public issh_known_hosts
{
public:
	explicit mock_ssh_known_hosts();
	~mock_ssh_known_hosts() override;

	MOCK_METHOD(result, verify, (const std::string& host, const std::string& pubkey_hash), (override));
	MOCK_METHOD(void, persist, (const std::string& host, const std::string& pubkey_hash), (override));
};

// This regex search and replace patterns can help to convert declarations into MOCK_METHOD macro calls
// Search pattern: ^(.+)\s([a-zA-Z_][a-zA-Z0-9_]*)\s*(\([^)]*\))\s*override\s*;\s*$
// Replace pattern: MOCK_METHOD(\1,\2,\3,(override));

using nice_mock_ssh_known_hosts   = testing::NiceMock<mock_ssh_known_hosts>;
using naggy_mock_ssh_known_hosts  = testing::NaggyMock<mock_ssh_known_hosts>;
using strict_mock_ssh_known_hosts = testing::StrictMock<mock_ssh_known_hosts>;

} // namespace sftp
} // namespace fs
} // namespace zoo
