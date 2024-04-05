//
// Copyright (C) 2022-2024 Patrick Rotsaert
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "zoo/common/api.h"
#include "zoo/squid/postgresql/connection.h"
#include "zoo/squid/postgresql/detail/libpqfwd.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#include <memory>
#include <string>
#include <functional>

namespace zoo {
namespace squid {
namespace postgresql {

class ZOO_EXPORT notify_listener final : public std::enable_shared_from_this<notify_listener>
{
	using callback_type     = std::function<void(const std::string& channel, int pid)>;
	using stream_descriptor = boost::asio::posix::stream_descriptor;

	stream_descriptor stream_;
	connection        connection_;
	callback_type     callback_;
	PGconn&           native_conn_;

	void on_wait(const boost::system::error_code& ec);
	void async_wait();

public:
	notify_listener(boost::asio::io_context& ioc, connection&& connection, const std::string& channel, callback_type&& callback);

	void run();
};

} // namespace postgresql
} // namespace squid
} // namespace zoo

#include "notify_listener.ipp"
