#include "zoo/tui/asioinput.h"
#include "zoo/tui/asiotimer.h"
#include "zoo/tui/keycode.h"
#include "zoo/tui/keymodifier.h"

#include "zoo/common/logging/logging.h"

#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/optional/optional.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <optional>
#include <string_view>
#include <array>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstdint>

namespace zoo {
namespace tui {

namespace {

template<typename Iterator>
auto dump(Iterator begin, Iterator end)
{
	fmt::memory_buffer buf{};
	for (auto it = begin; it < end; ++it)
	{
		const auto c = *it;
		if (c == 27)
		{
			fmt::format_to(std::back_inserter(buf), "ESC ");
		}
		else if (c == -1)
		{
			fmt::format_to(std::back_inserter(buf), "NOCHAR ");
		}
		else if (std::isprint(c))
		{
			fmt::format_to(std::back_inserter(buf), "'{:c}' ", c);
		}
		else
		{
			fmt::format_to(std::back_inserter(buf), "{:d} ", c & 0xff);
		}
	}
	return fmt::to_string(buf);
}

template<typename Iterator>
boost::optional<std::vector<int>> parse_number_list(Iterator begin, Iterator end)
{
	auto result = std::vector<int>{};
	enum
	{
		NONE,
		NUMBER,
		SEMICOLON
	} state = NONE;
	int number{};
	for (auto it = begin; it < end; ++it)
	{
		const auto c = *it;
		if (c == ';')
		{
			switch (state)
			{
			case NONE:
				ZOO_LOG(warn, "Unexpected initial ';");
				return boost::none;
			case NUMBER:
				state = SEMICOLON;
				result.push_back(number);
				number = 0;
				break;
			case SEMICOLON:
				ZOO_LOG(warn, "Unexpected consecutive ';");
				return boost::none;
			}
		}
		else if (std::isdigit(c))
		{
			state = NUMBER;
			number *= 10;
			number += (c - '0');
		}
	}
	if (state == NUMBER)
	{
		result.push_back(number);
	}
	return result;
}

std::vector<key_modifier> translate_key_modifiers(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end)
{
	auto result = std::vector<key_modifier>{};
	for (auto it = begin; it < end; ++it)
	{
		auto number = *it;
		if (number < 1)
		{
			ZOO_LOG(warn, "Invalid modifier {}", number);
			continue;
		}
		--number;
		if (number & 1)
		{
			result.push_back(key_modifier::SHIFT);
		}
		if (number & 2)
		{
			result.push_back(key_modifier::ALT);
		}
		if (number & 4)
		{
			result.push_back(key_modifier::CONTROL);
		}
		if (number & 8)
		{
			result.push_back(key_modifier::META);
		}
	}
	return result;
}

} // namespace

class asio_input::impl final
{
	static constexpr int     NOCHAR_TIMEOUT = 200;
	static constexpr int16_t NOCHAR         = -1;

	boost::asio::io_context&              ioc_;
	boost::asio::posix::stream_descriptor input_;
	asio_timer                            timer_;
	std::function<void(int)>              key_pressed_handler_;

	std::array<char, 16> read_buffer_{};
	std::vector<int16_t> input_buffer_{};

	std::unique_ptr<boost::asio::signal_set> ctrl_c_;

	using Iterator = std::vector<int16_t>::iterator;

	void fail(std::string_view what, boost::system::error_code ec)
	{
		ZOO_LOG(err, "{}: {}", what, ec.message());
	}

	void async_read()
	{
		this->input_.async_read_some(boost::asio::buffer(this->read_buffer_),
		                             std::bind(&impl::on_read, this, std::placeholders::_1, std::placeholders::_2));
	}

	void on_read(boost::system::error_code ec, std::size_t length)
	{
		if (ec)
		{
			return this->fail("read", ec);
		}

		ZOO_LOG(trace, "read: {}", dump(this->read_buffer_.begin(), this->read_buffer_.begin() + length));

		for (std::size_t i = 0; i < length; ++i)
		{
			this->input_buffer_.push_back(static_cast<int16_t>(this->read_buffer_[i]) & 0xff);
		}

		this->timer_.cancel();

		this->async_read();

		this->process_input_buffer();
	}

	void process_input_buffer()
	{
		auto&& erase = [this](int count) -> void {
			this->input_buffer_.erase(this->input_buffer_.begin(), this->input_buffer_.begin() + count);
		};

		auto&& start_timer_lambda = [this]() -> void {
			this->timer_.start(NOCHAR_TIMEOUT, [this]() {
				this->input_buffer_.push_back(NOCHAR);
				this->process_input_buffer();
			});
		};

		ZOO_LOG(trace, "buff: {}", dump(this->input_buffer_.begin(), this->input_buffer_.end()));

		while (!this->input_buffer_.empty())
		{
			const auto first = this->input_buffer_.at(0u);
			if (first == 27)
			{
				if (this->input_buffer_.size() > 1)
				{
					const auto second = this->input_buffer_.at(1u);
					if (second == NOCHAR)
					{
						// { <esc> <nochar> } -> esc
						this->key_pressed(key_code::ESC);
						erase(2);
					}
					else if (second == 27)
					{
						// { <esc> } <esc> -> esc
						this->key_pressed(key_code::ESC);
						erase(1);
					}
					else if (second == '[' || second == 'O')
					{
						if (this->input_buffer_.size() > 2)
						{
							const auto third = this->input_buffer_.at(2u);
							if (third == NOCHAR)
							{
								// { <esc> [ <nochar> } -> Alt+[
								// { <esc> O <nochar> } -> Alt+O
								this->key_pressed(second, { key_modifier::ALT });
								erase(3);
							}
							else if (third == 27)
							{
								// { <esc> [ } <esc> -> Alt+[
								// { <esc> O } <esc> -> Alt+O
								this->key_pressed(second, { key_modifier::ALT });
								erase(2);
							}
							else
							{
								auto begin = this->input_buffer_.begin() + 2;
								if (third == '[' && second == '[')
								{
									++begin;
								}
								const auto it = std::find_if(
								    begin, this->input_buffer_.end(), [](int16_t c) { return (c >= 0x40 && c <= 0x7e) || c == NOCHAR; });
								if (it == this->input_buffer_.end())
								{
									return start_timer_lambda();
								}
								else if (*it == NOCHAR)
								{
									this->process_escape_sequence(this->input_buffer_.begin(), it);
									this->input_buffer_.erase(this->input_buffer_.begin(), it + 1);
								}
								else
								{
									this->process_escape_sequence(this->input_buffer_.begin(), it + 1);
									this->input_buffer_.erase(this->input_buffer_.begin(), it + 1);
								}
							}
						}
						else
						{
							return start_timer_lambda();
						}
					}
					else
					{
						// <esc> <char> is Alt+char
						this->key_pressed(second, { key_modifier::ALT });
						erase(2);
					}
				}
				else
				{
					return start_timer_lambda();
				}
			}
			else
			{
				this->process_key(first);
				erase(1);
			}
		}
	}

	void process_key(int16_t key)
	{
		// BACKSPACE and Ctrl+H both generate ASCII 8
		if (key == 8 || key == 127)
		{
			return this->key_pressed(key_code::BACKSPACE);
		}

		// TAB and Ctrl+I both generate ASCII 9
		if (key == '\t')
		{
			return this->key_pressed(key_code::TAB);
		}

		// ENTER and Ctrl+J both generate ASCII 10
		if (key == '\n')
		{
			return this->key_pressed(key_code::ENTER);
		}

		// Ctrl+] -> 29
		if (key == 29)
		{
			return this->key_pressed(']', { key_modifier::CONTROL });
		}

		// Ctrl+A -> 1
		// Ctrl+B -> 2
		// ...
		// Ctrl-Z -> 26
		if (key >= ('A' - 'A' + 1) && key <= ('Z' - 'A' + 1))
		{
			this->key_pressed('A' + key - 1, { key_modifier::CONTROL });
		}
		else
		{
			this->key_pressed(key);
		}
	}

	void process_escape_sequence(Iterator begin, Iterator end)
	{
		assert(std::distance(begin, end) >= 3);
		assert(*begin == 27);

		const auto second = *(begin + 1);
		assert(second == '[' || second == 'O');

		const auto third = *(begin + 2);

		ZOO_LOG(trace, "seq: {}", dump(begin, end));

		const auto opt_numbers = parse_number_list(begin + 2, end - 1);
		if (!opt_numbers.has_value())
		{
			ZOO_LOG(warn,
			        "Invalid escape sequence {}\n"
			        "Invalid number list.",
			        dump(begin, end));
			return;
		}
		const auto& numbers = opt_numbers.value();
		ZOO_LOG(trace, "numbers: {}", numbers);

		const auto last = *(end - 1);

		// Linux console generates ESC [ [ LETTER for F1 to F5
		if (second == '[' && third == '[' && std::isalpha(last))
		{
			auto code = key_code{};
			switch (last)
			{
			case 'A':
				code = key_code::F1;
				break;
			case 'B':
				code = key_code::F2;
				break;
			case 'C':
				code = key_code::F3;
				break;
			case 'D':
				code = key_code::F4;
				break;
			case 'E':
				code = key_code::F5;
				break;
			default:
				ZOO_LOG(warn, "Unknown console key code '{}'", last);
				return;
			}
			this->key_pressed(code, translate_key_modifiers(numbers.begin(), numbers.end()));
		}
		// VT sequences
		else if (second == '[' && last == '~')
		{
			if (numbers.empty())
			{
				ZOO_LOG(warn,
				        "Invalid escape sequence {}\n"
				        "At least one number expected.",
				        dump(begin, end));
				return;
			}
			auto code = key_code{};
			switch (numbers.front())
			{
			case 1:
				code = key_code::HOME;
				break;
			case 2:
				code = key_code::INS;
				break;
			case 3:
				code = key_code::DEL;
				break;
			case 4:
				code = key_code::END;
				break;
			case 5:
				code = key_code::PGUP;
				break;
			case 6:
				code = key_code::PGDOWN;
				break;
			case 7:
				code = key_code::HOME;
				break;
			case 8:
				code = key_code::END;
				break;
			case 10:
				code = key_code::F0;
				break;
			case 11:
				code = key_code::F1;
				break;
			case 12:
				code = key_code::F2;
				break;
			case 13:
				code = key_code::F3;
				break;
			case 14:
				code = key_code::F4;
				break;
			case 15:
				code = key_code::F5;
				break;
			case 17:
				code = key_code::F6;
				break;
			case 18:
				code = key_code::F7;
				break;
			case 19:
				code = key_code::F8;
				break;
			case 20:
				code = key_code::F9;
				break;
			case 21:
				code = key_code::F10;
				break;
			case 23:
				code = key_code::F11;
				break;
			case 24:
				code = key_code::F12;
				break;
			case 25:
				code = key_code::F13;
				break;
			case 26:
				code = key_code::F14;
				break;
			case 28:
				code = key_code::F15;
				break;
			case 29:
				code = key_code::F16;
				break;
			case 31:
				code = key_code::F17;
				break;
			case 32:
				code = key_code::F18;
				break;
			case 33:
				code = key_code::F19;
				break;
			case 34:
				code = key_code::F20;
				break;
			default:
				ZOO_LOG(warn, "Unknown VT key code {}", numbers.front());
				return;
			}
			this->key_pressed(code, translate_key_modifiers(numbers.begin() + 1, numbers.end()));
		}
		// XTERM sequences
		else if ((second == '[' || second == 'O') && std::isupper(last))
		{
			auto code = key_code{};
			switch (last)
			{
			case 'A':
				code = key_code::UP;
				break;
			case 'B':
				code = key_code::DOWN;
				break;
			case 'C':
				code = key_code::RIGHT;
				break;
			case 'D':
				code = key_code::LEFT;
				break;
			case 'F':
				code = key_code::END;
				break;
			case 'G':
				code = key_code::KEYPAD_5;
				break;
			case 'H':
				code = key_code::HOME;
				break;
			case 'P':
				code = key_code::F1;
				break;
			case 'Q':
				code = key_code::F2;
				break;
			case 'R':
				code = key_code::F3;
				break;
			case 'S':
				code = key_code::F4;
				break;
			case 'Z':
				return this->key_pressed(key_code::TAB, { key_modifier::SHIFT });
			default:
				ZOO_LOG(warn, "Unknown xterm key code '{}'", last);
				return;
			}
			this->key_pressed(code, translate_key_modifiers(numbers.begin(), numbers.end()));
		}
		else
		{
			ZOO_LOG(warn,
			        "Invalid escape sequence {}\n"
			        "Second-last combination not recognized.",
			        dump(begin, end));
		}
	}

	template<typename T>
	void key_pressed(const T& code)
	{
		if (this->key_pressed_handler_)
		{
			this->key_pressed_handler_(static_cast<int>(code));
		}
	}

	template<typename T>
	void key_pressed(const T& code, const std::vector<key_modifier>& modifiers)
	{
		if (this->key_pressed_handler_)
		{
			auto key = static_cast<int>(code);
			for (const auto m : modifiers)
			{
				key |= static_cast<int>(m);
			}
			this->key_pressed_handler_(key);
		}
	}

public:
	explicit impl(boost::asio::io_context& ioc, int fd)
	    : ioc_{ ioc }
	    , input_{ ioc, fd }
	    , timer_{ ioc }
	    , key_pressed_handler_{}
	    , ctrl_c_{}
	{
		this->async_read();
	}

	void set_key_pressed_handler(std::function<void(int)> handler)
	{
		this->key_pressed_handler_ = std::move(handler);
	}

	void set_trap_ctrl_c(bool on)
	{
		if (on && !this->ctrl_c_)
		{
			this->ctrl_c_ = std::make_unique<boost::asio::signal_set>(this->ioc_, SIGINT);
			this->ctrl_c_->async_wait([&](const auto&, int) { this->key_pressed('C', { key_modifier::CONTROL }); });
		}
		else if (!on && this->ctrl_c_)
		{
			this->ctrl_c_->cancel();
			this->ctrl_c_.reset();
		}
	}
};

asio_input::asio_input(boost::asio::io_context& ioc, int fd)
    : pimpl_{ std::make_unique<impl>(ioc, fd) }
{
}

asio_input::~asio_input() = default;

void asio_input::set_trap_ctrl_c(bool on)
{
	return this->pimpl_->set_trap_ctrl_c(on);
}

void asio_input::set_key_pressed_handler(std::function<void(int)> handler)
{
	return this->pimpl_->set_key_pressed_handler(std::move(handler));
}

} // namespace tui
} // namespace zoo
