#pragma once

#include <boost/asio/io_context.hpp>

#include <memory>
#include <functional>

namespace zoo {
namespace tui {

class asio_timer final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit asio_timer(boost::asio::io_context& ioc);
	~asio_timer() noexcept;

	void start(int msec, std::function<void()> work);
	void cancel();
};

} // namespace tui
} // namespace zoo
