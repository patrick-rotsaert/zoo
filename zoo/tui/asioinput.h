#pragma once

#include <boost/asio/io_context.hpp>

#include <functional>
#include <memory>

namespace zoo {
namespace tui {

class asio_input final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit asio_input(boost::asio::io_context& ioc, int fd);
	~asio_input();

	void set_trap_ctrl_c(bool on);
	void set_key_pressed_handler(std::function<void(int key)> handler);
};

} // namespace tui
} // namespace zoo
