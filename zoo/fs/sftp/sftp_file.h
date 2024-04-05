//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/sftp/sftp_session.h"
#include "zoo/fs/sftp/issh_api.h"
#include "zoo/fs/core/ifile.h"
#include "zoo/fs/core/iinterruptor.h"
#include "zoo/fs/core/fspath.h"
#include "zoo/common/api.h"
#include <memory>
#include <cstddef>

namespace zoo {
namespace fs {
namespace sftp {

class ZOO_EXPORT file final : public ifile
{
	issh_api*                     api_;
	sftp_file                      fd_;
	fspath                         path_;
	std::shared_ptr<session>       session_;
	std::shared_ptr<iinterruptor> interruptor_;

public:
	explicit file(issh_api*                     api,
	              sftp_file                      fd,
	              const fspath&                  path,
	              std::shared_ptr<session>       session,
	              std::shared_ptr<iinterruptor> interruptor);
	~file() noexcept;

	std::size_t read(void* buf, std::size_t count) override;
	std::size_t write(const void* buf, std::size_t count) override;
};

} // namespace sftp
} // namespace fs
} // namespace zoo
