//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/sftp_session.h"
#include "zoo/fs/sftp/sftp_exceptions.h"
#include "zoo/fs/sftp/ssh_connection.h"
#include "zoo/fs/sftp/ssh_server_pubkey.h"
#include "zoo/fs/sftp/ssh_pubkey_hash.h"
#include "zoo/fs/sftp/ssh_private_key.h"
#include "zoo/fs/sftp/ssh_public_key.h"
#include "zoo/fs/sftp/ptr_types.h"
#include "zoo/fs/core/exceptions.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/logging/log_level.h"
#include "zoo/common/misc/formatters.hpp"
#include <fmt/format.h>
#include <cassert>
#include <libssh/callbacks.h>

namespace zoo {
namespace fs {
namespace sftp {

namespace {

void ssh_logging_callback(int priority, const char* function, const char* buffer, void* /*userdata*/)
{
	using logging::log_level;
	auto& backend = logging::logging::backend;
	if (!backend)
	{
		return;
	}

	auto lvl = log_level{};
	switch (priority)
	{
	case SSH_LOG_NONE:
		lvl = log_level::off;
		break;
	case SSH_LOG_WARN:
		lvl = log_level::warn;
		break;
	case SSH_LOG_INFO:
		lvl = log_level::info;
		break;
	case SSH_LOG_DEBUG:
		lvl = log_level::debug;
		break;
	case SSH_LOG_TRACE:
		lvl = log_level::trace;
		break;
	default:
		return;
	}

	backend->log_message(
	    std::chrono::system_clock::now(), boost::source_location{ nullptr, 0, function }, lvl, fmt::format("[ssh] {}", buffer));
}

int convert_ssh_logging_verbosity(options::ssh_log_level in)
{
	switch (in)
	{
	case options::ssh_log_level::NOLOG:
		return SSH_LOG_NOLOG;
	case options::ssh_log_level::WARNING:
		return SSH_LOG_WARNING;
	case options::ssh_log_level::PROTOCOL:
		return SSH_LOG_PROTOCOL;
	case options::ssh_log_level::PACKET:
		return SSH_LOG_PACKET;
	case options::ssh_log_level::FUNCTIONS:
		return SSH_LOG_FUNCTIONS;
	}
	ZOO_THROW_EXCEPTION(should_not_happen_exception{});
}

} // namespace

class session::impl final
{
	issh_api*                              api_;
	options                                opts_;
	std::shared_ptr<issh_known_hosts>      known_hosts_;
	std::shared_ptr<issh_identity_factory> ssh_identity_factory_;
	std::shared_ptr<iinterruptor>          interruptor_;
	ssh_session_ptr                        ssh_;
	std::unique_ptr<ssh_connection>        connection_;
	sftp_session_ptr                       sftp_;

	static void connect_status_callback(void* userdata, float status)
	{
		//zlog(trace,"connect status {}", status);
		(void)status;

		auto self = reinterpret_cast<impl*>(userdata);
		self->interruptor_->throw_if_interrupted();
	}

	auto setup_ssh()
	{
		auto ssh = ssh_session_ptr{ this->api_->ssh_new(), [this](auto s) {
			                           if (s)
			                           {
				                           this->api_->ssh_free(s);
			                           }
			                       } };
		if (!ssh)
		{
			ZOO_THROW_EXCEPTION(ssh_exception("ssh_new() returned nullptr") << error_opname{ "ssh_new" });
		}

		auto cb                    = ssh_callbacks_struct{};
		cb.userdata                = this;
		cb.connect_status_function = impl::connect_status_callback;
		ssh_callbacks_init(&cb);
		this->api_->ssh_set_callbacks(ssh.get(), &cb);

		// CONNECT
		zlog(debug, "connect host={}, port={}", this->opts_.host, this->opts_.port);

		this->api_->ssh_options_set(ssh.get(), SSH_OPTIONS_HOST, this->opts_.host.c_str());

		if (this->opts_.port)
		{
			this->api_->ssh_options_set(ssh.get(), SSH_OPTIONS_PORT, &(this->opts_.port.value()));
		}

		const auto verbosity = convert_ssh_logging_verbosity(this->opts_.ssh_logging_verbosity);
		this->api_->ssh_options_set(ssh.get(), SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

		auto connection = std::make_unique<ssh_connection>(this->api_, ssh);

		this->interruptor_->throw_if_interrupted();

		// VERIFY HOST KEY
		zlog(debug, "verify host key");

		{
			const auto pubkey = ssh_server_pubkey{ this->api_, ssh.get() };
			const auto hash   = ssh_pubkey_hash{ this->api_, ssh.get(), pubkey.key().get(), SSH_PUBLICKEY_HASH_SHA1 };

			switch (this->known_hosts_->verify(this->opts_.host, hash.hash()))
			{
			case issh_known_hosts::result::KNOWN:
				zlog(debug, "host is known");
				break;
			case issh_known_hosts::result::UNKNOWN:
				zlog(debug, "host is unknown");
				if (this->opts_.allow_unknown_host_key)
				{
					this->known_hosts_->persist(this->opts_.host, hash.hash());
				}
				else
				{
					ZOO_THROW_EXCEPTION(ssh_host_key_unknown("Unknown SSH host")
					                    << ssh_host_key_unknown::ssh_host(this->opts_.host)
					                    << ssh_host_key_unknown::ssh_host_pubkey_hash(hash.hash()));
				}
				break;
			case issh_known_hosts::result::CHANGED:
				zlog(warn, "host key changed");
				if (this->opts_.allow_changed_host_key)
				{
					this->known_hosts_->persist(this->opts_.host, hash.hash());
				}
				else
				{
					ZOO_THROW_EXCEPTION(ssh_host_key_changed("SSH host key changed")
					                    << ssh_host_key_changed::ssh_host(this->opts_.host)
					                    << ssh_host_key_changed::ssh_host_pubkey_hash(hash.hash()));
				}
				break;
			}
		}

		this->interruptor_->throw_if_interrupted();

		// AUTHENTICATE
		zlog(debug, "authenticate user {}", this->opts_.user);

		{
			// NOTE: ssh_userauth_list requires the function ssh_userauth_none() to be called before the methods are available.
			if (this->api_->ssh_userauth_none(ssh.get(), nullptr) == SSH_AUTH_ERROR)
			{
				ZOO_THROW_EXCEPTION(ssh_exception("Unexpected error") << error_opname{ "ssh_userauth_none" });
			}

			const auto methods       = this->api_->ssh_userauth_list(ssh.get(), nullptr);
			auto       authenticated = false;

			if (!authenticated && (methods & SSH_AUTH_METHOD_NONE))
			{
				this->interruptor_->throw_if_interrupted();

				zlog(trace, "attempting NONE authentication");
				const auto rc = this->api_->ssh_userauth_none(ssh.get(), this->opts_.user.c_str());
				if (rc == SSH_AUTH_SUCCESS)
				{
					zlog(info, "NONE authentication successful");
					authenticated = true;
				}
				else
				{
					zlog(err, "NONE authentication failed: {}", this->api_->ssh_get_error(ssh.get()));
				}
			}

			if (!authenticated && (methods & SSH_AUTH_METHOD_PUBLICKEY))
			{
				for (const auto& identity : this->ssh_identity_factory_->create())
				{
					this->interruptor_->throw_if_interrupted();

					zlog(debug, "attempting public key authentication with identity '{}'", identity->name);
					const auto pkey   = ssh_private_key{ this->api_, identity->pkey };
					const auto pubkey = ssh_public_key{ this->api_, pkey };
					auto       rc     = this->api_->ssh_userauth_try_publickey(ssh.get(), nullptr, pubkey.key().get());
					if (rc == SSH_AUTH_SUCCESS)
					{
						rc = this->api_->ssh_userauth_publickey(ssh.get(), this->opts_.user.c_str(), pkey.key().get());
						if (rc == SSH_AUTH_SUCCESS)
						{
							zlog(info, "public key authentication successful with identity '{}'", identity->name);
							authenticated = true;
							break;
						}
						else if (rc == SSH_AUTH_ERROR)
						{
							ZOO_THROW_EXCEPTION(ssh_auth_exception(this->api_, ssh.get()) << error_opname{ "ssh_userauth_publickey" });
						}
						else
						{
							// do not throw, this error is probably not fatal and another auth method might still succeed
							zlog(err, "Public key authentication failed: {}", this->api_->ssh_get_error(ssh.get()));
						}
					}
					else if (rc == SSH_AUTH_DENIED)
					{
						zlog(debug, "server refused public key");
					}
					else if (rc == SSH_AUTH_ERROR)
					{
						ZOO_THROW_EXCEPTION(ssh_auth_exception(this->api_, ssh.get()) << error_opname{ "ssh_userauth_try_publickey" });
					}
					else
					{
						// do not throw, this error is probably not fatal and another auth method might still succeed
						zlog(err, "Public key authentication failed: {}", this->api_->ssh_get_error(ssh.get()));
					}
				}
			}

			if (!authenticated && (methods & SSH_AUTH_METHOD_PASSWORD) && this->opts_.password)
			{
				this->interruptor_->throw_if_interrupted();

				zlog(trace, "attempting password authentication");
				const auto rc = this->api_->ssh_userauth_password(ssh.get(), this->opts_.user.c_str(), this->opts_.password->c_str());
				if (rc == SSH_AUTH_SUCCESS)
				{
					zlog(info, "password authentication successful");
					authenticated = true;
				}
				else
				{
					zlog(err, "Password authentication failed: {}", this->api_->ssh_get_error(ssh.get()));
				}
			}

			if (!authenticated)
			{
				ZOO_THROW_EXCEPTION(ssh_auth_exception("None of the offered authentication methods were successful"));
			}
		}

		return std::make_tuple(ssh, std::move(connection));
	}

	auto setup_sftp()
	{
		this->interruptor_->throw_if_interrupted();

		auto ssh = this->ssh();

		// SFTP session
		zlog(debug, "create sftp session");

		const auto sftp = sftp_session_ptr{ this->api_->sftp_new(ssh), [this](auto s) {
			                                   if (s)
			                                   {
				                                   this->api_->sftp_free(s);
			                                   }
			                               } };
		if (!sftp)
		{
			ZOO_THROW_EXCEPTION(ssh_exception("sftp_new() returned nullptr") << error_opname{ "sftp_new" });
		}

		const auto rc = this->api_->sftp_init(sftp.get());
		if (rc != SSH_OK)
		{
			ZOO_THROW_EXCEPTION(sftp_exception(this->api_, ssh, sftp.get()) << error_opname{ "sftp_init" });
		}

		return sftp;
	}

public:
	explicit impl(issh_api*                              api,
	              const options&                         opts,
	              std::shared_ptr<issh_known_hosts>      known_hosts,
	              std::shared_ptr<issh_identity_factory> ssh_identity_factory,
	              std::shared_ptr<iinterruptor>          interruptor,
	              bool                                   lazy)
	    : api_{ api }
	    , opts_{ opts }
	    , known_hosts_{ known_hosts }
	    , ssh_identity_factory_{ ssh_identity_factory }
	    , interruptor_{ interruptor }
	    , ssh_{}
	    , connection_{}
	    , sftp_{}
	{
		this->api_->ssh_set_log_callback(ssh_logging_callback);

		if (!lazy)
		{
			(void)this->sftp();
		}
	}

	~impl()
	{
		this->sftp_.reset();
		this->connection_.reset();
		this->ssh_.reset();
	}

	ssh_session ssh()
	{
		if (!this->ssh_)
		{
			auto [ssh, connection] = this->setup_ssh();
			this->connection_      = std::move(connection);
			this->ssh_             = ssh;
		}
		return this->ssh_.get();
	}

	sftp_session sftp()
	{
		if (!this->sftp_)
		{
			this->sftp_ = this->setup_sftp();
		}
		return this->sftp_.get();
	}
};

session::session(issh_api*                              api,
                 const options&                         opts,
                 std::shared_ptr<issh_known_hosts>      known_hosts,
                 std::shared_ptr<issh_identity_factory> ssh_identity_factory,
                 std::shared_ptr<iinterruptor>          interruptor,
                 bool                                   lazy)
    : pimpl_{ std::make_unique<impl>(api, opts, known_hosts, ssh_identity_factory, interruptor, lazy) }
{
}

session::session(session&& src) = default;

session& session::operator=(session&& src) = default;

session::~session() noexcept = default;

ssh_session session::ssh()
{
	return this->pimpl_->ssh();
}

sftp_session session::sftp()
{
	return this->pimpl_->sftp();
}

} // namespace sftp
} // namespace fs
} // namespace zoo
