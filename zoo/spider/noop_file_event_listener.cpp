//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "zoo/spider/noop_file_event_listener.h"

namespace zoo {
namespace spider {

noop_file_event_listener::noop_file_event_listener()
{
}

noop_file_event_listener::~noop_file_event_listener()
{
}

void noop_file_event_listener::on_open(const char*, file_mode, const error_code&)
{
}

void noop_file_event_listener::on_close(const error_code&, std::uint64_t, std::uint64_t)
{
}

void noop_file_event_listener::on_seek(std::uint64_t, const error_code&)
{
}

void noop_file_event_listener::on_read(void*, std::size_t, const error_code&)
{
}

void noop_file_event_listener::on_write(const void*, std::size_t, const error_code&)
{
}

} // namespace spider
} // namespace zoo
