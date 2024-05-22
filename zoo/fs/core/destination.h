//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/fs/core/config.h"
#include "zoo/fs/core/fspath.h"
#include <optional>

namespace zoo {
namespace fs {

class ZOO_FS_CORE_API destination final
{
public:
	enum class conflict_policy
	{
		OVERWRITE,
		AUTORENAME,
		FAIL
	};

	enum class time_expansion
	{
		UTC,
		LOCAL
	};

	fspath path;                                            // The destination path.
	                                                        // --
	std::optional<time_expansion> expand_time_placeholders; // Expands time placeholders using the mtime of the source file.
	                                                        // See https://fmt.dev/latest/syntax.html#chrono-specs
	                                                        // The value controls if UTC or local time is used as input.
	                                                        // If not set (std::nullopt), then no time expansion takes place.
	                                                        // --
	bool create_parents;                                    // Recursively create parent directories of `path`?
	                                                        // --
	conflict_policy on_name_conflict;                       // How to handle an existing destination file.

	/// The destination path (this->path) is constructed from @a path as follows:
	///
	/// If @a expand_time_placeholders is set, time placeholders are expanded as described above.
	/// If the resulting path exists:
	///   If it is a directory:
	///     Append the original filename of @a source to the path.
	///     If the resulting path exists:
	///       If it is a directory:
	///         Throw error.
	///       Else:
	///         Resolve name conflict.
	///   Else if the path ends with a path separator:
	///     Throw error.
	///   Else:
	///     Resolve name conflict.
	/// Else:
	///   If the path ends with a path separator:
	///     Append the original filename of @a source to the path.
	///   If the path has a parent path:
	///     If @a create_parents is true:
	///       Recursively create the parent path.
	///
	/// Name conflict resulution is controlled by @a on_name_conflict:
	/// - OVERWRITE: The existing file will be overwritten.
	/// - FAIL: The construction will fail.
	/// - AUTORENAME:
	///     A new name is formed as {stem}~{i}{ext}, where:
	///     {stem} is the filename stem, i.e. "file" in "file.txt"
	///     {ext} is the filename extension, i.e. ".txt" in "file.txt"
	///     {i} is incremented starting from 1 until the new name does not exist.
	///     Example: For a desination filename "file.txt", where "file.txt" exists,
	///     the new name will be "file~1.txt". If that also exists, then "file~2.txt"
	///     and so on.
	///     Note that race conditions may occur if another thread or process
	///     creates the same file after this constructor returns.
	destination(const fspath&                        path,
	            const std::optional<time_expansion>& expand_time_placeholders,
	            bool                                 create_parents,
	            conflict_policy                      on_name_conflict);
};

} // namespace fs
} // namespace zoo
