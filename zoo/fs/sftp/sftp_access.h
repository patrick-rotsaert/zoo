//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/sftp_options.h"
#include "zoo/fs/sftp/issh_knownhosts.h"
#include "zoo/fs/sftp/issh_identity_factory.h"
#include "zoo/fs/core/iaccess.h"
#include "zoo/fs/core/iinterruptor.h"
#include "zoo/common/api.h"
#include <optional>
#include <memory>

namespace zoo {
namespace fs {
namespace sftp {

class issh_api;

class ZOO_EXPORT access final : public iaccess
{
	class impl;
	std::shared_ptr<impl> pimpl_;

public:
	explicit access(const options&                         opts,
	                std::shared_ptr<issh_known_hosts>      known_hosts,
	                std::shared_ptr<issh_identity_factory> ssh_identity_factory,
	                std::shared_ptr<iinterruptor>          interruptor,
	                bool                                   lazy = false);
	explicit access(issh_api&                              api,
	                const options&                         opts,
	                std::shared_ptr<issh_known_hosts>      known_hosts,
	                std::shared_ptr<issh_identity_factory> ssh_identity_factory,
	                std::shared_ptr<iinterruptor>          interruptor,
	                bool                                   lazy = false);
	~access() noexcept;

	access(access&&)            = default;
	access& operator=(access&&) = default;

	access(const access&)            = delete;
	access& operator=(const access&) = delete;

	bool                      is_remote() const override;
	std::vector<direntry>     ls(const fspath& dir) override;
	bool                      exists(const fspath& path) override;
	std::optional<attributes> try_stat(const fspath& path) override;
	attributes                stat(const fspath& path) override;
	std::optional<attributes> try_lstat(const fspath& path) override;
	attributes                lstat(const fspath& path) override;
	void                      remove(const fspath& path) override;
	void                      mkdir(const fspath& path, bool parents) override;
	void                      rename(const fspath& oldpath, const fspath& newpath) override;
	std::unique_ptr<ifile>    open(const fspath& path, int flags, mode_t mode) override;
	std::shared_ptr<iwatcher> create_watcher(const fspath& dir, int cancelfd) override;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
