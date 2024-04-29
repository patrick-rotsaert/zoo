//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/issh_api.h"
#include "zoo/fs/sftp/config.h"
#include <gmock/gmock.h>
#include <array>
#include <string>

namespace zoo {
namespace fs {
namespace sftp {

class ZOO_FS_SFTP_API mock_ssh_api : public issh_api
{
public:
	static ssh_session                   test_ssh_session;
	static ssh_key                       test_server_publickey;
	static std::array<unsigned char, 20> test_server_publickey_hash;
	static std::string                   test_server_publickey_hash_hexa;
	static ssh_key                       test_ident_pkey;
	static ssh_key                       test_ident_pubkey;
	static sftp_session                  test_sftp_session;
	static sftp_dir                      test_sftp_dir;
	static sftp_attributes               test_sftp_attributes;
	static sftp_file                     test_sftp_file;

	explicit mock_ssh_api();
	~mock_ssh_api() override;

	MOCK_METHOD(ssh_session, ssh_new, (), (override));
	MOCK_METHOD(void, ssh_free, (ssh_session session), (override));
	MOCK_METHOD(int, ssh_options_set, (ssh_session session, enum ssh_options_e type, const void* value), (override));
	MOCK_METHOD(int, ssh_connect, (ssh_session session), (override));
	MOCK_METHOD(void, ssh_disconnect, (ssh_session session), (override));
	MOCK_METHOD(int, ssh_get_server_publickey, (ssh_session session, ssh_key* key), (override));
	MOCK_METHOD(int,
	            ssh_get_publickey_hash,
	            (const ssh_key key, enum ssh_publickey_hash_type type, unsigned char** hash, size_t* hlen),
	            (override));
	MOCK_METHOD(void, ssh_clean_pubkey_hash, (unsigned char** hash), (override));
	MOCK_METHOD(int,
	            ssh_pki_import_privkey_base64,
	            (const char* b64_key, const char* passphrase, ssh_auth_callback auth_fn, void* auth_data, ssh_key* pkey),
	            (override));
	MOCK_METHOD(int, ssh_pki_export_privkey_to_pubkey, (const ssh_key privkey, ssh_key* pkey), (override));
	MOCK_METHOD(int, ssh_key_is_private, (const ssh_key k), (override));
	MOCK_METHOD(int, ssh_key_is_public, (const ssh_key k), (override));
	MOCK_METHOD(void, ssh_key_free, (ssh_key key), (override));
	MOCK_METHOD(int, ssh_userauth_none, (ssh_session session, const char* username), (override));
	MOCK_METHOD(int, ssh_userauth_list, (ssh_session session, const char* username), (override));
	MOCK_METHOD(int, ssh_userauth_try_publickey, (ssh_session session, const char* username, const ssh_key pubkey), (override));
	MOCK_METHOD(int, ssh_userauth_publickey, (ssh_session session, const char* username, const ssh_key privkey), (override));
	MOCK_METHOD(int, ssh_userauth_password, (ssh_session session, const char* username, const char* password), (override));
	MOCK_METHOD(int, ssh_set_log_callback, (ssh_logging_callback cb), (override));
	MOCK_METHOD(int, ssh_set_callbacks, (ssh_session session, ssh_callbacks cb), (override));
	MOCK_METHOD(const char*, ssh_get_error, (void* error), (override));
	MOCK_METHOD(int, ssh_get_error_code, (void* error), (override));
	MOCK_METHOD(char*, ssh_get_hexa, (const unsigned char* what, size_t len), (override));
	MOCK_METHOD(void, ssh_string_free_char, (char* s), (override));

	MOCK_METHOD(sftp_session, sftp_new, (ssh_session session), (override));
	MOCK_METHOD(void, sftp_free, (sftp_session sftp), (override));
	MOCK_METHOD(int, sftp_init, (sftp_session sftp), (override));
	MOCK_METHOD(sftp_attributes, sftp_stat, (sftp_session session, const char* path), (override));
	MOCK_METHOD(sftp_attributes, sftp_lstat, (sftp_session session, const char* path), (override));
	MOCK_METHOD(char*, sftp_readlink, (sftp_session sftp, const char* path), (override));
	MOCK_METHOD(sftp_dir, sftp_opendir, (sftp_session session, const char* path), (override));
	MOCK_METHOD(int, sftp_closedir, (sftp_dir dir), (override));
	MOCK_METHOD(sftp_attributes, sftp_readdir, (sftp_session session, sftp_dir dir), (override));
	MOCK_METHOD(sftp_file, sftp_open, (sftp_session session, const char* file, int accesstype, mode_t mode), (override));
	MOCK_METHOD(int, sftp_close, (sftp_file file), (override));
	MOCK_METHOD(ssize_t, sftp_read, (sftp_file file, void* buf, size_t count), (override));
	MOCK_METHOD(ssize_t, sftp_write, (sftp_file file, const void* buf, size_t count), (override));
	MOCK_METHOD(int, sftp_mkdir, (sftp_session sftp, const char* directory, mode_t mode), (override));
	MOCK_METHOD(int, sftp_rename, (sftp_session sftp, const char* original, const char* newname), (override));
	MOCK_METHOD(int, sftp_unlink, (sftp_session sftp, const char* file), (override));
	MOCK_METHOD(void, sftp_attributes_free, (sftp_attributes file), (override));
	MOCK_METHOD(int, sftp_dir_eof, (sftp_dir dir), (override));
	MOCK_METHOD(int, sftp_get_error, (sftp_session sftp), (override));
};

// This regex search and replace patterns can help to convert declarations into MOCK_METHOD macro calls
// Search pattern: ^(.+)\s([a-zA-Z_][a-zA-Z0-9_]*)\s*(\([^)]*\))\s*override\s*;\s*$
// Replace pattern: MOCK_METHOD(\1,\2,\3,(override));

using nice_mock_ssh_api   = testing::NiceMock<mock_ssh_api>;
using naggy_mock_ssh_api  = testing::NaggyMock<mock_ssh_api>;
using strict_mock_ssh_api = testing::StrictMock<mock_ssh_api>;

} // namespace sftp
} // namespace fs
} // namespace zoo
