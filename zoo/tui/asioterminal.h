#pragma once

#include "zoo/tui/tuiattribute.h"
#include "zoo/tui/tuisize.h"
#include "zoo/tui/tuiposition.h"
#include "zoo/tui/config.h"

#include <boost/asio/io_context.hpp>
#include <boost/utility/string_view.hpp>

#include <memory>
#include <functional>

namespace zoo {
namespace tui {

using u32_string_view = boost::basic_string_view<char32_t, std::char_traits<char32_t>>;
using string_view      = boost::basic_string_view<char, std::char_traits<char>>;

class ZOO_TUI_API asio_terminal final
{
	class impl;
	std::unique_ptr<impl> pimpl_;

public:
	explicit asio_terminal(boost::asio::io_context& ioc);
	~asio_terminal() noexcept;

	// The value for `key` is a key_code enum (see keycode.h) or character,
	// possibly bitwised OR-er with one or more key_modifier enums (see keymodifier.h).
	void set_key_pressed_handler(std::function<void(int key)> handler);
	void set_trap_ctrl_c(bool on);

	void set_window_size_changed_handler(std::function<void()> handler);

	tui_size size() const;
	void     cursor(bool on);
	void     cls(tui_attribute::type attr);
	void     print(tui_attribute::type attr, int x, int y, string_view s);
	void     print(tui_attribute::type attr, int x, int y, u32_string_view s);
	void     print(tui_attribute::type attr, int x, int y, char32_t c);
	void     print(tui_attribute::type attr, const tui_position& pos, string_view s);
	void     print(tui_attribute::type attr, const tui_position& pos, u32_string_view s);
	void     print(tui_attribute::type attr, const tui_position& pos, char32_t c);
	void     horizontal_line(tui_attribute::type attr, const tui_position& pos, int width);
	void     horizontal_line(tui_attribute::type attr, const tui_position& pos, int width, char32_t c);
	void     vertical_line(tui_attribute::type attr, const tui_position& pos, int height);
	void     vertical_line(tui_attribute::type attr, const tui_position& pos, int height, char32_t c);

	void update();
};

} // namespace tui
} // namespace zoo
