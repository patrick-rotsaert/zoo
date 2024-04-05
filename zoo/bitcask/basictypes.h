//
// Copyright (C) 2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <string>
#include <cstdint>

#include <sys/types.h>

namespace zoo {
namespace bitcask {

using crc_type       = std::uint32_t;
using version_type   = std::uint64_t;
using ksz_type       = std::uint32_t;
using value_sz_type  = std::uint32_t;
using value_pos_type = off64_t;

using file_id_type = std::uint64_t;

using key_type   = std::string;
using value_type = std::string;

constexpr auto file_id_bits    = sizeof(file_id_type) * 8u;
constexpr auto file_id_nibbles = sizeof(file_id_type) * 2u;

} // namespace bitcask
}
