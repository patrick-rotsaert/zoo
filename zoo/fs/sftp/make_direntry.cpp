//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/fs/sftp/make_direntry.h"
#include "zoo/fs/sftp/make_attributes.h"
#include "zoo/fs/sftp/sftp_exceptions.h"
#include "zoo/fs/core/exceptions.h"
#include "zoo/common/logging/logging.h"
#include "zoo/common/misc/formatters.hpp"

namespace zoo {
namespace fs {
namespace sftp {

direntry make_direntry(issh_api* api, const fspath& path, sftp_session sftp, const sftp_attributes a)
{
	assert(a != nullptr);
	auto entry = direntry{};
	entry.name = a->name;
	entry.attr = make_attributes(a);
	if (entry.attr.is_lnk())
	{
		zlog(trace, "sftp_readlink path={}", path);
		const auto target = api->sftp_readlink(sftp, path.string().c_str());
		if (target)
		{
			entry.symlink_target = target;
			free(target);
		}
		else
		{
			ZOO_THROW_EXCEPTION(sftp_exception(api, sftp) << error_opname{ "sftp_readlink" } << error_path{ path });
		}
	}

	return entry;
}

} // namespace sftp
} // namespace fs
} // namespace zoo
