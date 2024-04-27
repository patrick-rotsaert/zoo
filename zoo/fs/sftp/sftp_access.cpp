//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/sftp_access.h"
#include "zoo/fs/sftp/sftp_exceptions.h"
#include "zoo/fs/sftp/sftp_file.h"
#include "zoo/fs/sftp/sftp_watcher.h"
#include "zoo/fs/sftp/sftp_session.h"
#include "zoo/fs/sftp/ssh_api.h"
#include "zoo/fs/sftp/make_direntry.h"
#include "zoo/fs/sftp/make_attributes.h"
#include "zoo/fs/core/direntry.h"
#include "zoo/fs/core/attributes.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"
#include <optional>
#include <type_traits>
#include <chrono>
#include <cassert>
#include <cstddef>
#include <libssh/sftp.h>

namespace zoo {
namespace fs {
namespace sftp {

namespace {

using sftp_attributes_ptr = std::shared_ptr<std::remove_pointer<sftp_attributes>::type>;

class directory_reader final
{
	issh_api*                api_;
	std::shared_ptr<session> session_;
	fspath                   path_;
	sftp_dir                 dir_;
	struct dirent*           entry_;

public:
	explicit directory_reader(issh_api* api, std::shared_ptr<session> session, const fspath& path)
	    : api_{ api }
	    , session_{ session }
	    , path_{ path }
	{
		zlog(trace, "sftp_opendir path={}", path);
		this->dir_ = this->api_->sftp_opendir(session->sftp(), path.string().c_str());
		zlog(trace, "dir {}", fmt::ptr(this->dir_));

		if (!this->dir_)
		{
			ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_) << error_opname{ "sftp_opendir" } << error_path{ path });
		}
	}

	~directory_reader() noexcept
	{
		zlog(trace, "sftp_closedir {}", fmt::ptr(this->dir_));
		this->api_->sftp_closedir(this->dir_);
	}

	std::optional<direntry> read()
	{
		zlog(trace, "sftp_readdir {}", fmt::ptr(this->dir_));
		const auto attrib = this->api_->sftp_readdir(this->session_->sftp(), this->dir_);
		if (attrib)
		{
			const auto guard = sftp_attributes_ptr{ attrib, [this](auto a) {
				                                       if (a)
				                                       {
					                                       this->api_->sftp_attributes_free(a);
				                                       }
				                                   } };
			assert(attrib->name);
			return make_direntry(this->api_, this->path_ / attrib->name, this->session_->sftp(), attrib);
		}
		else
		{
			if (this->api_->sftp_dir_eof(this->dir_))
			{
				return std::nullopt;
			}
			else
			{
				ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_)
				                    << error_opname{ "sftp_readdir" } << error_path{ this->path_ });
			}
		}
	}
};

} // namespace

class access::impl final : public iaccess, public std::enable_shared_from_this<impl>
{
	issh_api*                     api_;
	std::shared_ptr<iinterruptor> interruptor_;
	std::shared_ptr<session>      session_;
	std::uint32_t                 watcher_scan_interval_ms_;

public:
	explicit impl(issh_api*                              api,
	              const options&                         opts,
	              std::shared_ptr<issh_known_hosts>      known_hosts,
	              std::shared_ptr<issh_identity_factory> ssh_identity_factory,
	              std::shared_ptr<iinterruptor>          interruptor,
	              bool                                   lazy)
	    : api_{ api }
	    , interruptor_{ interruptor }
	    , session_{ std::make_shared<session>(api, opts, known_hosts, ssh_identity_factory, interruptor, lazy) }
	    , watcher_scan_interval_ms_{ opts.watcher_scan_interval_ms }
	{
		zlog(trace, "sftp access: host={}, port={}, user={}", opts.host, opts.port, opts.user);
	}

	impl(impl&&)            = delete;
	impl& operator=(impl&&) = delete;

	impl(const impl&)            = delete;
	impl& operator=(const impl&) = delete;

	bool is_remote() const override
	{
		return true;
	}

	std::vector<direntry> ls(const fspath& dir) override
	{
		this->interruptor_->throw_if_interrupted();

		auto result = std::vector<direntry>{};
		auto dr     = directory_reader{ this->api_, this->session_, dir };
		while (auto entry = dr.read())
		{
			this->interruptor_->throw_if_interrupted();
			result.push_back(std::move(entry.value()));
		}
		return result;
	}

	bool exists(const fspath& path) override
	{
		this->interruptor_->throw_if_interrupted();

		zlog(trace, "sftp_stat path={}", path);
		const auto attrib = this->api_->sftp_stat(this->session_->sftp(), path.string().c_str());
		if (attrib)
		{
			this->api_->sftp_attributes_free(attrib);
			return true;
		}
		else
		{
			const auto err = this->api_->sftp_get_error(this->session_->sftp());
			if (err == SSH_FX_NO_SUCH_FILE)
			{
				return false;
			}
			else
			{
				ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_) << error_opname{ "sftp_stat" } << error_path{ path });
			}
		}
	}

	std::optional<attributes> try_stat(const fspath& path) override
	{
		this->interruptor_->throw_if_interrupted();

		zlog(trace, "sftp_stat path={}", path);
		const auto attrib = this->api_->sftp_stat(this->session_->sftp(), path.string().c_str());
		if (attrib)
		{
			const auto guard = sftp_attributes_ptr{ attrib, [this](auto a) {
				                                       if (a)
				                                       {
					                                       this->api_->sftp_attributes_free(a);
				                                       }
				                                   } };
			return make_attributes(attrib);
		}
		else
		{
			const auto err = this->api_->sftp_get_error(this->session_->sftp());
			if (err == SSH_FX_NO_SUCH_FILE)
			{
				return std::nullopt;
			}
			else
			{
				ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_) << error_opname{ "sftp_stat" } << error_path{ path });
			}
		}
	}

	attributes stat(const fspath& path) override
	{
		this->interruptor_->throw_if_interrupted();

		zlog(trace, "sftp_stat path={}", path);
		const auto attrib = this->api_->sftp_stat(this->session_->sftp(), path.string().c_str());
		if (attrib)
		{
			const auto guard = sftp_attributes_ptr{ attrib, [this](auto a) {
				                                       if (a)
				                                       {
					                                       this->api_->sftp_attributes_free(a);
				                                       }
				                                   } };
			return make_attributes(attrib);
		}
		else
		{
			ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_) << error_opname{ "sftp_stat" } << error_path{ path });
		}
	}

	std::optional<attributes> try_lstat(const fspath& path) override
	{
		this->interruptor_->throw_if_interrupted();

		zlog(trace, "sftp_lstat path={}", path);
		const auto attrib = this->api_->sftp_lstat(this->session_->sftp(), path.string().c_str());
		if (attrib)
		{
			const auto guard = sftp_attributes_ptr{ attrib, [this](auto a) {
				                                       if (a)
				                                       {
					                                       this->api_->sftp_attributes_free(a);
				                                       }
				                                   } };
			return make_attributes(attrib);
		}
		else
		{
			const auto err = this->api_->sftp_get_error(this->session_->sftp());
			if (err == SSH_FX_NO_SUCH_FILE)
			{
				return std::nullopt;
			}
			else
			{
				ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_) << error_opname{ "sftp_stat" } << error_path{ path });
			}
		}
	}

	attributes lstat(const fspath& path) override
	{
		this->interruptor_->throw_if_interrupted();

		zlog(trace, "sftp_lstat path={}", path);
		const auto attrib = this->api_->sftp_lstat(this->session_->sftp(), path.string().c_str());
		if (attrib)
		{
			const auto guard = sftp_attributes_ptr{ attrib, [this](auto a) {
				                                       if (a)
				                                       {
					                                       this->api_->sftp_attributes_free(a);
				                                       }
				                                   } };
			return make_attributes(attrib);
		}
		else
		{
			ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_) << error_opname{ "sftp_lstat" } << error_path{ path });
		}
	}

	void remove(const fspath& path) override
	{
		this->interruptor_->throw_if_interrupted();

		zlog(trace, "sftp_unlink path={}", path);
		if (this->api_->sftp_unlink(this->session_->sftp(), path.string().c_str()) < 0)
		{
			ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_) << error_opname{ "sftp_unlink" } << error_path{ path });
		}
	}

	void mkdir(const fspath& path, bool parents) override
	{
		this->interruptor_->throw_if_interrupted();

		zlog(trace, "sftp_mkdir path={}", path);
		if (this->api_->sftp_mkdir(this->session_->sftp(), path.string().c_str(), 0777) < 0)
		{
			if (parents && path.has_parent_path() && this->api_->sftp_get_error(this->session_->sftp()) == SSH_FX_NO_SUCH_FILE)
			{
				this->mkdir(path.parent_path(), true);
				this->mkdir(path, false);
				return;
			}
			ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_) << error_opname{ "sftp_mkdir" } << error_path{ path });
		}
	}

	void rename(const fspath& oldpath, const fspath& newpath) override
	{
		this->interruptor_->throw_if_interrupted();

		zlog(trace, "sftp_rename oldpath={} newpath={}", oldpath, newpath);
		if (this->api_->sftp_rename(this->session_->sftp(), oldpath.string().c_str(), newpath.string().c_str()) < 0)
		{
			ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_)
			                    << error_opname{ "sftp_rename" } << error_oldpath{ oldpath } << error_newpath{ newpath });
		}
	}

	std::unique_ptr<ifile> open(const fspath& path, int flags, mode_t mode) override
	{
		this->interruptor_->throw_if_interrupted();

		zlog(trace, "sftp_open path={} flags={:o} mode={:o}", path, flags, mode);
		const auto fd = this->api_->sftp_open(this->session_->sftp(), path.string().c_str(), flags, mode);
		zlog(trace, "fd {}", static_cast<void*>(fd));
		if (fd == nullptr)
		{
			ZOO_THROW_EXCEPTION(sftp_exception(this->api_, this->session_) << error_opname{ "sftp_open" } << error_path{ path });
		}
		else
		{
			return std::make_unique<file>(this->api_, fd, path, this->session_, this->interruptor_);
		}
	}

	std::shared_ptr<iwatcher> create_watcher(const fspath& dir) override
	{
		return std::make_shared<watcher>(dir, this->watcher_scan_interval_ms_, this->shared_from_this(), this->interruptor_);
	}
};

namespace {
ssh_api g_api;
}

access::access(const options&                         opts,
               std::shared_ptr<issh_known_hosts>      known_hosts,
               std::shared_ptr<issh_identity_factory> ssh_identity_factory,
               std::shared_ptr<iinterruptor>          interruptor,
               bool                                   lazy)
    : pimpl_{ std::make_shared<impl>(&g_api, opts, known_hosts, ssh_identity_factory, interruptor, lazy) }
{
}

access::access(issh_api&                              api,
               const options&                         opts,
               std::shared_ptr<issh_known_hosts>      known_hosts,
               std::shared_ptr<issh_identity_factory> ssh_identity_factory,
               std::shared_ptr<iinterruptor>          interruptor,
               bool                                   lazy)
    : pimpl_{ std::make_shared<impl>(&api, opts, known_hosts, ssh_identity_factory, interruptor, lazy) }
{
}

access::~access() noexcept = default;

bool access::is_remote() const
{
	return this->pimpl_->is_remote();
}

std::vector<direntry> access::ls(const fspath& dir)
{
	return this->pimpl_->ls(dir);
}

bool access::exists(const fspath& path)
{
	return this->pimpl_->exists(path);
}

std::optional<attributes> access::try_stat(const fspath& path)
{
	return this->pimpl_->try_stat(path);
}

attributes access::stat(const fspath& path)
{
	return this->pimpl_->stat(path);
}

std::optional<attributes> access::try_lstat(const fspath& path)
{
	return this->pimpl_->try_lstat(path);
}

attributes access::lstat(const fspath& path)
{
	return this->pimpl_->lstat(path);
}

void access::remove(const fspath& path)
{
	return this->pimpl_->remove(path);
}

void access::mkdir(const fspath& path, bool parents)
{
	return this->pimpl_->mkdir(path, parents);
}

void access::rename(const fspath& oldpath, const fspath& newpath)
{
	return this->pimpl_->rename(oldpath, newpath);
}

std::unique_ptr<ifile> access::open(const fspath& path, int flags, mode_t mode)
{
	return this->pimpl_->open(path, flags, mode);
}

std::shared_ptr<iwatcher> access::create_watcher(const fspath& dir)
{
	return this->pimpl_->create_watcher(dir);
}

} // namespace sftp
} // namespace fs
} // namespace zoo
