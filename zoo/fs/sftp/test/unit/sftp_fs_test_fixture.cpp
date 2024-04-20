//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "sftp_fs_test_fixture.h"

namespace zoo {
namespace fs {
namespace sftp {

SftpFsTestFixture::SftpFsTestFixture()
{
	this->identity->name = "ident_name";
	this->identity->pkey = "ident_pkey";
}

std::vector<std::shared_ptr<ssh_identity>> SftpFsTestFixture::make_ssh_idents()
{
	return { this->identity };
}

bool SftpFsTestFixture::setup_ssh_calls(issh_known_hosts::result knownhosts_verify_result,
                                        int                      user_auth_method,
                                        ssh_auth_e               auth_method_result,
                                        ssh_auth_e               auth_method2_result)
{
	auto sq           = testing::InSequence{};
	auto expect_throw = false;
	EXPECT_CALL(this->nice_ssh_api, ssh_new()).Times(1).WillOnce(testing::Return(mock_ssh_api::test_ssh_session));
	EXPECT_CALL(this->nice_ssh_api, ssh_options_set(mock_ssh_api::test_ssh_session, SSH_OPTIONS_HOST, testing::NotNull()))
	    .Times(1)
	    .WillOnce(testing::DoAll([this](ssh_session,
	                                    enum ssh_options_e,
	                                    const void* value) { EXPECT_STREQ(reinterpret_cast<const char*>(value), this->opts.host.c_str()); },
	                             testing::Return(SSH_OK)));
	if (this->opts.port.has_value())
	{
		EXPECT_CALL(this->nice_ssh_api, ssh_options_set(mock_ssh_api::test_ssh_session, SSH_OPTIONS_PORT, testing::NotNull()))
		    .Times(1)
		    .WillOnce(testing::DoAll(
		        [this](ssh_session, enum ssh_options_e, const void* value) {
			        EXPECT_EQ(*static_cast<const std::uint16_t*>(value), this->opts.port.value());
		        },
		        testing::Return(SSH_OK)));
	}
	EXPECT_CALL(this->nice_ssh_api, ssh_options_set(mock_ssh_api::test_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, testing::NotNull()))
	    .Times(1)
	    .WillOnce(testing::Return(SSH_OK));
	EXPECT_CALL(this->nice_ssh_api, ssh_connect(mock_ssh_api::test_ssh_session)).Times(1).WillOnce(testing::Return(SSH_OK));
	EXPECT_CALL(this->nice_ssh_api, ssh_get_server_publickey(mock_ssh_api::test_ssh_session, testing::NotNull()))
	    .Times(1)
	    .WillOnce(testing::DoAll([](ssh_session, ssh_key* key) { *key = mock_ssh_api::test_server_publickey; }, testing::Return(0)));
	EXPECT_CALL(
	    this->nice_ssh_api,
	    ssh_get_publickey_hash(mock_ssh_api::test_server_publickey, SSH_PUBLICKEY_HASH_SHA1, testing::NotNull(), testing::NotNull()))
	    .Times(1)
	    .WillOnce(testing::DoAll(
	        [](const ssh_key, enum ssh_publickey_hash_type, unsigned char** hash, size_t* hlen) {
		        *hash = mock_ssh_api::test_server_publickey_hash.data();
		        *hlen = mock_ssh_api::test_server_publickey_hash.size();
	        },
	        testing::SaveArg<2>(&this->saved_hash),
	        testing::Return(0)));
	EXPECT_CALL(this->nice_ssh_api,
	            ssh_get_hexa(mock_ssh_api::test_server_publickey_hash.data(), mock_ssh_api::test_server_publickey_hash.size()))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_server_publickey_hash_hexa.data()));
	EXPECT_CALL(this->nice_ssh_api, ssh_clean_pubkey_hash(testing::_)).Times(1).WillOnce(testing::DoAll([&](unsigned char** hash) {
		EXPECT_EQ(hash, this->saved_hash);
	}));
	EXPECT_CALL(this->nice_ssh_api, ssh_string_free_char(testing::Eq(mock_ssh_api::test_server_publickey_hash_hexa.data()))).Times(1);
	EXPECT_CALL(*this->nice_ssh_known_hosts,
	            verify(testing::StrEq(this->opts.host), testing::StrEq(mock_ssh_api::test_server_publickey_hash_hexa)))
	    .Times(1)
	    .WillOnce(testing::Return(knownhosts_verify_result));
	switch (knownhosts_verify_result)
	{
	case issh_known_hosts::result::KNOWN:
		break;
	case issh_known_hosts::result::UNKNOWN:
		if (this->opts.allow_unknown_host_key)
		{
			EXPECT_CALL(*this->nice_ssh_known_hosts,
			            persist(testing::StrEq(this->opts.host), testing::StrEq(mock_ssh_api::test_server_publickey_hash_hexa)))
			    .Times(1);
		}
		else
		{
			expect_throw = true;
		}
		break;
	case issh_known_hosts::result::CHANGED:
		if (this->opts.allow_changed_host_key)
		{
			EXPECT_CALL(*this->nice_ssh_known_hosts,
			            persist(testing::StrEq(this->opts.host), testing::StrEq(mock_ssh_api::test_server_publickey_hash_hexa)))
			    .Times(1);
		}
		else
		{
			expect_throw = true;
		}
		break;
	}
	EXPECT_CALL(this->nice_ssh_api, ssh_key_free(mock_ssh_api::test_server_publickey)).Times(1);
	if (!expect_throw)
	{
		EXPECT_CALL(this->nice_ssh_api, ssh_userauth_none(mock_ssh_api::test_ssh_session, nullptr))
		    .Times(1)
		    .WillOnce(testing::Return(SSH_AUTH_AGAIN));
		EXPECT_CALL(this->nice_ssh_api, ssh_userauth_list(mock_ssh_api::test_ssh_session, nullptr))
		    .Times(1)
		    .WillOnce(testing::Return(user_auth_method));
		if (user_auth_method & SSH_AUTH_METHOD_NONE)
		{
			EXPECT_CALL(this->nice_ssh_api, ssh_userauth_none(mock_ssh_api::test_ssh_session, testing::StrEq(this->opts.user.c_str())))
			    .Times(1)
			    .WillOnce(testing::Return(auth_method_result));
			if (auth_method_result != SSH_AUTH_SUCCESS)
			{
				expect_throw = true;
			}
		}
		if (user_auth_method & SSH_AUTH_METHOD_PUBLICKEY)
		{
			EXPECT_CALL(*this->nice_ssh_identity_factory, create()).Times(1).WillOnce(testing::Return(this->make_ssh_idents()));
			EXPECT_CALL(this->nice_ssh_api,
			            ssh_pki_import_privkey_base64(testing::StrEq(this->identity->pkey.c_str()),
			                                          testing::IsNull(),
			                                          testing::IsNull(),
			                                          testing::IsNull(),
			                                          testing::NotNull()))
			    .Times(1)
			    .WillOnce(testing::DoAll(
			        [](const char*, const char*, ssh_auth_callback, void*, ssh_key* pkey) { *pkey = mock_ssh_api::test_ident_pkey; },
			        testing::Return(SSH_OK)));
			EXPECT_CALL(this->nice_ssh_api, ssh_key_is_private(mock_ssh_api::test_ident_pkey)).Times(1).WillOnce(testing::Return(true));
			EXPECT_CALL(this->nice_ssh_api, ssh_pki_export_privkey_to_pubkey(mock_ssh_api::test_ident_pkey, testing::NotNull()))
			    .Times(1)
			    .WillOnce(
			        testing::DoAll([](const ssh_key, ssh_key* pkey) { *pkey = mock_ssh_api::test_ident_pubkey; }, testing::Return(SSH_OK)));
			EXPECT_CALL(this->nice_ssh_api, ssh_key_is_public(mock_ssh_api::test_ident_pubkey)).Times(1).WillOnce(testing::Return(true));
			EXPECT_CALL(this->nice_ssh_api,
			            ssh_userauth_try_publickey(mock_ssh_api::test_ssh_session, testing::IsNull(), mock_ssh_api::test_ident_pubkey))
			    .Times(1)
			    .WillOnce(testing::Return(auth_method_result));
			if (auth_method_result != SSH_AUTH_SUCCESS)
			{
				expect_throw = true;
			}
			else
			{
				EXPECT_CALL(this->nice_ssh_api,
				            ssh_userauth_publickey(
				                mock_ssh_api::test_ssh_session, testing::StrEq(this->opts.user.c_str()), mock_ssh_api::test_ident_pkey))
				    .Times(1)
				    .WillOnce(testing::Return(auth_method2_result));
				if (auth_method2_result != SSH_AUTH_SUCCESS)
				{
					expect_throw = true;
				}
			}
			EXPECT_CALL(this->nice_ssh_api, ssh_key_free(mock_ssh_api::test_ident_pubkey)).Times(1);
			EXPECT_CALL(this->nice_ssh_api, ssh_key_free(mock_ssh_api::test_ident_pkey)).Times(1);
		}
		if (user_auth_method & SSH_AUTH_METHOD_PASSWORD)
		{
			EXPECT_CALL(this->nice_ssh_api,
			            ssh_userauth_password(mock_ssh_api::test_ssh_session,
			                                  testing::StrEq(this->opts.user.c_str()),
			                                  testing::StrEq(this->opts.password->c_str())))
			    .Times(1)
			    .WillOnce(testing::Return(auth_method_result));
			if (auth_method_result != SSH_AUTH_SUCCESS)
			{
				expect_throw = true;
			}
		}
	}
	EXPECT_CALL(this->nice_ssh_api, ssh_disconnect(mock_ssh_api::test_ssh_session)).Times(1);
	EXPECT_CALL(this->nice_ssh_api, ssh_free(mock_ssh_api::test_ssh_session)).Times(1);
	return !expect_throw;
}

void SftpFsTestFixture::setup_sftp_calls()
{
	EXPECT_TRUE(this->setup_ssh_calls(issh_known_hosts::result::KNOWN, SSH_AUTH_METHOD_NONE, SSH_AUTH_SUCCESS));
	EXPECT_CALL(this->nice_ssh_api, sftp_new(mock_ssh_api::test_ssh_session))
	    .Times(1)
	    .WillOnce(testing::Return(mock_ssh_api::test_sftp_session));
	EXPECT_CALL(this->nice_ssh_api, sftp_init(mock_ssh_api::test_sftp_session)).Times(1).WillOnce(testing::Return(SSH_OK));
	EXPECT_CALL(this->nice_ssh_api, sftp_free(mock_ssh_api::test_sftp_session)).Times(1);
}

} // namespace sftp
} // namespace fs
} // namespace zoo
