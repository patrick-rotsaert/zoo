//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/local/config.h"
#include "zoo/fs/core/ifile.h"
#include "zoo/fs/core/iinterruptor.h"
#include "zoo/fs/core/fspath.h"

namespace zoo {
namespace fs {
namespace local {

class ZOO_FS_LOCAL_API file final : public ifile
{
	int                           fd_;
	fspath                        path_;
	std::shared_ptr<iinterruptor> interruptor_;

public:
	explicit file(int fd, const fspath& path, std::shared_ptr<iinterruptor> interruptor);
	~file() noexcept;

	std::size_t read(void* buf, std::size_t count) override;
	std::size_t write(const void* buf, std::size_t count) override;
};

} // namespace local
} // namespace fs
} // namespace zoo
