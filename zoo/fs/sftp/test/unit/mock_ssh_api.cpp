//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "mock_ssh_api.h"

struct ssh_session_struct
{
};

struct ssh_key_struct
{
};

namespace zoo {
namespace fs {
namespace sftp {

namespace {

sftp_attributes make_test_sftp_attributes()
{
	static sftp_attributes_struct a{};
	static char                   name[] = "somefile";
	a.name                               = name;
	return &a;
}

ssh_session_struct  g_test_ssh_session{};
ssh_key_struct      g_test_server_publickey{}, g_test_ident_pkey{}, g_test_ident_pubkey{};
sftp_session_struct g_test_sftp_session;
sftp_dir_struct     g_test_sftp_dir;
sftp_file_struct    g_test_sftp_file;

} // namespace

ssh_session                   mock_ssh_api::test_ssh_session      = &g_test_ssh_session;
ssh_key                       mock_ssh_api::test_server_publickey = &g_test_server_publickey;
std::array<unsigned char, 20> mock_ssh_api::test_server_publickey_hash{};
std::string                   mock_ssh_api::test_server_publickey_hash_hexa{ "0123456789012345678901234567890123456789" };
ssh_key                       mock_ssh_api::test_ident_pkey      = &g_test_ident_pkey;
ssh_key                       mock_ssh_api::test_ident_pubkey    = &g_test_ident_pubkey;
sftp_session                  mock_ssh_api::test_sftp_session    = &g_test_sftp_session;
sftp_dir                      mock_ssh_api::test_sftp_dir        = &g_test_sftp_dir;
sftp_attributes               mock_ssh_api::test_sftp_attributes = make_test_sftp_attributes();
sftp_file                     mock_ssh_api::test_sftp_file       = &g_test_sftp_file;

mock_ssh_api::mock_ssh_api()
{
}

mock_ssh_api::~mock_ssh_api()
{
}

} // namespace sftp
} // namespace fs
} // namespace zoo
