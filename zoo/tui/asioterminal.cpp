#include "zoo/tui/asioterminal.h"
#include "zoo/tui/asioinput.h"
#include "zoo/tui/keycode.h"
#include "zoo/tui/keymodifier.h"
#include "zoo/tui/character.h"

#include "zoo/common/logging/logging.h"

#include "tuiconfig.h"

#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <boost/throw_exception.hpp>
#include <boost/system/system_error.hpp>
#include <boost/optional/optional.hpp>
#include <boost/locale.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>

#if defined(HAVE_TERM_H)
#include <term.h>
#endif
#if defined(HAVE_TERMIO_H)
#include <termio.h>
#endif
#if defined(HAVE_TERMIOS_H)
#include <termios.h>
#endif

#include <string_view>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <system_error>
#include <cctype>

#include <unistd.h>
#include <locale.h>
#include <langinfo.h>
#include <sys/ioctl.h>

#if defined(CURSES_HAVE_CURSES_H)
#include <curses.h>
#elif defined(CURSES_HAVE_NCURSES_H)
#include <ncurses.h>
#elif defined(CURSES_HAVE_NCURSES_NCURSES_H)
#include <ncursesw/curses.h>
#elif defined(CURSES_HAVE_NCURSES_CURSES_H)
#include <ncurses/curses.h>
#else
#error No curses header available
#endif

namespace zoo {
namespace tui {

namespace {

struct Cell
{
	tui_attribute::type a{};
	char32_t        c{ U' ' };

	void clear(tui_attribute::type attr)
	{
		this->c = ' ';
		this->a = attr;
	}

	bool operator==(const Cell& other) const
	{
		return this->a == other.a && this->c == other.c;
	}
};

struct Line
{
	std::vector<Cell> cells{};

	Line()
	{
		this->cells.resize(columns);
	}

	boost::optional<std::reference_wrapper<Cell>> cell(int x)
	{
		if (x >= 0 && x < static_cast<int>(this->cells.size()))
		{
			return std::ref(this->cells[x]);
		}
		else
		{
			return boost::none;
		}
	}

	void clear(tui_attribute::type attr)
	{
		for (auto& cell : this->cells)
		{
			cell.clear(attr);
		}
	}
};

struct Screen
{
	std::vector<Line> rows{};

	Screen()
	{
		this->rows.resize(lines);
	}

	boost::optional<std::reference_wrapper<Cell>> cell(int x, int y)
	{
		if (y >= 0 && y < static_cast<int>(this->rows.size()))
		{
			return this->rows[y].cell(x);
		}
		else
		{
			return boost::none;
		}
	}

	void clear(tui_attribute::type attr)
	{
		for (auto& row : this->rows)
		{
			row.clear(attr);
		}
	}
};

class IGlyphPrinter
{
public:
	explicit IGlyphPrinter()
	{
	}

	virtual ~IGlyphPrinter()
	{
	}

	virtual void init()                                        = 0;
	virtual void print(std::string& buffer, u32_string_view s) = 0;

	void print(std::string& buffer, char32_t c)
	{
		this->print(buffer, u32_string_view{ &c, 1 });
	}

protected:
	IGlyphPrinter(IGlyphPrinter&&)      = delete;
	IGlyphPrinter(const IGlyphPrinter&) = delete;

	IGlyphPrinter& operator=(IGlyphPrinter&&)      = delete;
	IGlyphPrinter& operator=(const IGlyphPrinter&) = delete;
};

class Utf8GlyphPrinter final : public IGlyphPrinter
{
public:
	explicit Utf8GlyphPrinter()
	    : IGlyphPrinter{}
	{
	}

	~Utf8GlyphPrinter() override
	{
	}

	void init() override
	{
	}

	void print(std::string& buffer, u32_string_view s) override
	{
		buffer.append(boost::locale::conv::utf_to_utf<char>(s.begin(), s.end()));
	}
};

class AltGlyphPrinter final : public IGlyphPrinter
{
	struct Glyph
	{
		char graph;
		char ascii;
	};

	using GlyphMap = std::map<char32_t, Glyph>;

	static const GlyphMap& map()
	{
		static GlyphMap _{
			{ character::GC_UARROW, { '-', '^' } },   // arrow pointing up
			{ character::GC_DARROW, { '.', 'v' } },   // arrow pointing down
			{ character::GC_RARROW, { '+', '>' } },   // arrow pointing right
			{ character::GC_LARROW, { ',', '<' } },   // arrow pointing left
			{ character::GC_BLOCK, { '0', '#' } },    // solid square block
			{ character::GC_CKBOARD, { 'a', ':' } },  // checker board (stipple)
			{ character::GC_PLMINUS, { 'g', '#' } },  // plus/minus
			{ character::GC_BOARD, { 'h', '#' } },    // board of squares
			{ character::GC_LRCORNER, { 'j', '+' } }, // lower right corner
			{ character::GC_URCORNER, { 'k', '+' } }, // upper right corner
			{ character::GC_ULCORNER, { 'l', '+' } }, // upper left corner
			{ character::GC_LLCORNER, { 'm', '+' } }, // lower left corner
			{ character::GC_PLUS, { 'n', '+' } },     // large plus or crossover
			{ character::GC_S1, { 'o', '-' } },       // scan line 1
			{ character::GC_HLINE, { 'q', '-' } },    // horizontal line
			{ character::GC_LTEE, { 't', '+' } },     // tee pointing right
			{ character::GC_RTEE, { 'u', '+' } },     // tee pointing left
			{ character::GC_BTEE, { 'v', '+' } },     // tee pointing up
			{ character::GC_TTEE, { 'w', '+' } },     // tee pointing down
			{ character::GC_VLINE, { 'x', '|' } },    // vertical line
			{ character::GC_BULLET, { '~', 'o' } },   // bullet
			{ character::GC_LEQUAL, { 'y', '<' } },   // less-than-or-equal-to
			{ character::GC_GEQUAL, { 'z', '>' } },   // greater-than-or-equal-to
			{ character::GC_DIAMOND, { '`', ' ' } },  // diamond
			{ character::GC_STERLING, { '}', 'f' } }, // UK pound sign
			{ character::GC_DEGREE, { 'f', '\\' } },  // degree symbol
			{ character::GC_PI, { '{', '*' } },       // greek pi
		};
		return _;
	}

	GlyphMap map_;
	bool     alt_charset_enabled_;
	bool     alt_charset_selected_;
	bool     supports_alt_charset_;

	void enable_alt_charset(std::string& buffer)
	{
		if (this->supports_alt_charset_ && !this->alt_charset_enabled_)
		{
			assert(ena_acs);
			buffer.append(ena_acs);
			this->alt_charset_enabled_ = true;
		}
	}

	void enter_alt_charset(std::string& buffer)
	{
		if (this->supports_alt_charset_ && !this->alt_charset_selected_)
		{
			this->enable_alt_charset(buffer);
			assert(enter_alt_charset_mode);
			buffer.append(enter_alt_charset_mode);
			this->alt_charset_selected_ = true;
		}
	}

	void exit_alt_charset(std::string& buffer)
	{
		if (this->supports_alt_charset_ && this->alt_charset_selected_)
		{
			this->enable_alt_charset(buffer);
			assert(exit_alt_charset_mode);
			buffer.append(exit_alt_charset_mode);
			this->alt_charset_selected_ = false;
		}
	}

public:
	explicit AltGlyphPrinter()
	    : IGlyphPrinter{}
	    , map_{}
	    , alt_charset_enabled_{}
	    , alt_charset_selected_{}
	    , supports_alt_charset_{}
	{
	}

	~AltGlyphPrinter() override
	{
	}

	void init() override
	{
		this->map_ = map();
		if (acs_chars && enter_alt_charset_mode && exit_alt_charset_mode && ena_acs)
		{
			this->supports_alt_charset_ = true;
			// terminal capable of drawing glyphs
			string_view acsChars{ acs_chars };
			for (auto& pair : this->map_)
			{
				auto&      glyph = pair.second;
				const auto vt100 = glyph.graph;
				glyph.graph      = '\0';
				for (size_t i = 0; i < acsChars.length() / 2; ++i)
				{
					if (acsChars[2 * i] == vt100)
					{
						glyph.graph = acsChars[2 * i + 1];
						break;
					}
				}
			}
		}
		else
		{
			// terminal *not* capable of drawing glyphs
			this->supports_alt_charset_ = false;
		}
	}

	void print(std::string& buffer, u32_string_view s) override
	{
		for (const auto c : s)
		{
			if (c > 31 && c < 128)
			{
				this->exit_alt_charset(buffer);
				buffer.append(1, static_cast<char>(c));
			}
			else
			{
				auto it = this->map_.find(c);
				if (it == this->map_.end())
				{
					this->exit_alt_charset(buffer);
					buffer.append(1, '?');
				}
				else
				{
					const auto& glyph = it->second;
					if (this->supports_alt_charset_)
					{
						if (glyph.graph)
						{
							this->enable_alt_charset(buffer);
							buffer.append(1, glyph.graph);
						}
						else
						{
							this->exit_alt_charset(buffer);
							buffer.append(1, glyph.ascii);
						}
					}
					else
					{
						buffer.append(1, glyph.ascii);
					}
				}
			}
		}
	}
};

class GlyphPrinterFactory final
{
public:
	static std::unique_ptr<IGlyphPrinter> create()
	{
		setlocale(LC_CTYPE, "");
		auto encoding = nl_langinfo(CODESET);
		if (encoding && string_view{ encoding } == "UTF-8")
		{
			return std::make_unique<Utf8GlyphPrinter>();
		}
		else
		{
			return std::make_unique<AltGlyphPrinter>();
		}
	}
};

class terminal_output final
{
	int                            fd_{ STDERR_FILENO };
	int                            columns_{}, rows_{};
	std::string                    buffer_{};
	std::unique_ptr<IGlyphPrinter> glyphPrinter_{ GlyphPrinterFactory::create() };

	template<typename... Args>
	bool tparm(char* cap, Args... args)
	{
		auto s = ::tparm(cap, args...);
		if (s)
		{
			this->buffer_.append(s);
			return true;
		}
		else
		{
			return false;
		}
	}

	void flush()
	{
		if (!this->buffer_.empty())
		{
			this->write(this->buffer_);
			this->buffer_.clear();
		}
	}

	void write(string_view s)
	{
		auto result = ::write(this->fd_, s.data(), s.length());
		if (result < 0)
		{
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::generic_category(), "write"));
		}
	}

	void update_size()
	{
		winsize wsz{};
		if (ioctl(this->fd(), TIOCGWINSZ, &wsz) == -1)
		{
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::generic_category(), "ioctl(TIOCGWINSZ)"));
		}

		lines = this->rows_ = wsz.ws_row;
		columns = this->columns_ = wsz.ws_col;
		ZOO_LOG(debug, "rows: {}, columns: {}", this->rows_, this->columns_);
	}

public:
	terminal_output()
	{
		// setup terminal, this causes the reading of terminfo database
		if (setupterm(0, this->fd_, 0) != OK)
		{
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::generic_category(), "setupterm"));
		}

		// test most basic capabilities
		if (!clear_screen || !cursor_address)
		{
			BOOST_THROW_EXCEPTION(std::runtime_error{ "Terminal too dumb!" });
		}

		this->update_size();

		this->tparm(enter_ca_mode);

		this->attribute(tui_attribute::BG_BLACK);
		this->cls();

		this->flush();

		this->glyphPrinter_->init();
	}

	~terminal_output() noexcept
	{
		try
		{
			// turn off text attributes
			this->tparm(exit_attribute_mode);

			// set cursor visible
			this->cursor(true);

			if (!this->tparm(exit_ca_mode))
			{
				this->cls();
			}

			this->flush();
		}
		catch (const std::exception& e)
		{
			ZOO_LOG(warn, "{}", e.what());
		}
	}

	int fd() const
	{
		return this->fd_;
	}

	std::string unbuffer()
	{
		return std::move(this->buffer_);
	}

	tui_size size() const
	{
		return tui_size{ this->rows_, this->columns_ };
	}

	void cls(void)
	{
		this->tparm(clear_screen);
	}

	void cursor(bool on)
	{
		this->tparm(on ? cursor_normal : cursor_invisible);
	}

	void movexy(int x, int y)
	{
		if (x >= 0 && y >= 0)
		{
			this->tparm(cursor_address, y, x);
		}
	}

	void attribute(tui_attribute::type attr)
	{
		constexpr int fcoltab[] = { 30, 34, 32, 36, 31, 35, 33, 37 };
		constexpr int bcoltab[] = { 40, 44, 42, 46, 41, 45, 43, 47 };

		this->buffer_.append(
		    fmt::format("\x1b"
		                "[0;{};{}",
		                fcoltab[attr & 7],
		                bcoltab[(attr >> 4) & 7]));
		if (attr & tui_attribute::FG_BRIGHT)
		{
			this->buffer_.append(";1");
		}
		if (attr & tui_attribute::FG_BLINK)
		{
			this->buffer_.append(";5");
		}
		this->buffer_.append("m");
	}

	void print(u32_string_view s)
	{
		this->glyphPrinter_->print(this->buffer_, s);
	}

	void print(char32_t c)
	{
		this->glyphPrinter_->print(this->buffer_, c);
	}

	void window_size_changed()
	{
		this->update_size();
	}
};

class terminal_input final
{
	int     fd_{ STDIN_FILENO };
	termios tty_{};

public:
	terminal_input()
	{
		// get terminal input attributes
		termios tty{};
		if (tcgetattr(this->fd_, &tty) < 0)
		{
			throw std::system_error(errno, std::generic_category(), "tcgetattr");
		}

		// make a copy to restore in destructor
		this->tty_ = tty;

		// Modify terminal settings for non-canonical mode and no echo
		tty.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
		tty.c_cc[VMIN]  = 1;             // Minimum number of characters to read
		tty.c_cc[VTIME] = 0;             // Timeout (0 means no timeout)

		// activate the new attributes
		if (tcsetattr(this->fd_, TCSANOW, &tty) < 0)
		{
			throw std::system_error(errno, std::generic_category(), "tcsetattr");
		}
	}

	~terminal_input() noexcept
	{
		// restore original terminal input attributes */
		tcsetattr(this->fd_, TCSANOW, &this->tty_);
	}

	int fd() const
	{
		return this->fd_;
	}
};

} // namespace

class asio_terminal::impl
{
	terminal_input                        terminal_input_;
	terminal_output                       terminal_output_;
	asio_input                            input_;
	boost::asio::posix::stream_descriptor output_;
	std::function<void()>                 window_size_changed_handler_;
	boost::asio::signal_set               winch_signal_;
	std::unique_ptr<Screen>               last_screen_;
	std::unique_ptr<Screen>               next_screen_;
	Screen                                current_screen_;
	bool                                  cursor_;
	bool                                  write_in_progress_;
	std::string                           frame_;

public:
	explicit impl(boost::asio::io_context& ioc)
	    : terminal_input_{}
	    , terminal_output_{}
	    , input_{ ioc, ::dup(terminal_input_.fd()) }
	    , output_{ ioc, ::dup(terminal_output_.fd()) }
	    , window_size_changed_handler_{}
	    , winch_signal_{ ioc, SIGWINCH }
	    , last_screen_{}
	    , next_screen_{}
	    , current_screen_{}
	    , cursor_{}
	    , write_in_progress_{}
	    , frame_{}
	{
		this->input_.set_key_pressed_handler(std::bind(&impl::key_pressed, this, std::placeholders::_1));
		this->asyncWaitWinch();
	}

	void set_key_pressed_handler(std::function<void(int key)> handler)
	{
		this->input_.set_key_pressed_handler(handler);
	}

	void set_trap_ctrl_c(bool on)
	{
		this->input_.set_trap_ctrl_c(on);
	}

	void set_window_size_changed_handler(std::function<void()> handler)
	{
		this->window_size_changed_handler_ = std::move(handler);
	}

	tui_size size() const
	{
		return this->terminal_output_.size();
	}

	void cursor(bool on)
	{
		this->cursor_ = on;
	}

	void cls(tui_attribute::type attr)
	{
		this->current_screen_.clear(attr);
	}

	void print(tui_attribute::type attr, int x, int y, u32_string_view s)
	{
		for (const auto c : s)
		{
			auto optCellRef = this->current_screen_.cell(x, y);
			if (optCellRef.has_value())
			{
				auto& cell = optCellRef.value().get();
				cell.a     = attr;
				cell.c     = c;
				++x;
			}
			else
			{
				break;
			}
		}
	}

	void print(tui_attribute::type attr, int x, int y, string_view s)
	{
		return this->print(attr, x, y, boost::locale::conv::utf_to_utf<char32_t>(s.begin(), s.end()));
	}

	void print(tui_attribute::type attr, int x, int y, char32_t c)
	{
		auto optCellRef = this->current_screen_.cell(x, y);
		if (optCellRef.has_value())
		{
			auto& cell = optCellRef.value().get();
			cell.a     = attr;
			cell.c     = c;
		}
	}

	void horizontal_line(tui_attribute::type attr, tui_position pos, int width, char32_t c)
	{
		while (width-- > 0)
		{
			this->print(attr, pos.x++, pos.y, c);
		}
	}

	void vertical_line(tui_attribute::type attr, tui_position pos, int height, char32_t c)
	{
		while (height-- > 0)
		{
			this->print(attr, pos.x, pos.y++, c);
		}
	}

	void update()
	{
		this->next_screen_ = std::make_unique<Screen>(this->current_screen_);
		if (!this->write_in_progress_)
		{
			this->write_next_screen(std::move(this->next_screen_));
		}
	}

private:
	void asyncWaitWinch()
	{
		this->winch_signal_.async_wait([&](const auto&, int sig) {
			ZOO_LOG(trace, "Caught signal {}", sig);
			this->window_size_changed();
			if (this->window_size_changed_handler_)
			{
				this->window_size_changed_handler_();
			}
			this->asyncWaitWinch();
		});
	}

	void window_size_changed()
	{
		this->terminal_output_.window_size_changed();
		this->current_screen_ = Screen{};
		this->next_screen_.reset();
		this->last_screen_.reset();
	}

	void write_next_screen(std::unique_ptr<Screen> screen)
	{
		this->frame_ = this->last_screen_ ? this->render_diff(*screen, *this->last_screen_) : this->render_full(*screen);
		boost::asio::async_write(this->output_,
		                         boost::asio::buffer(this->frame_),
		                         std::bind(&impl::on_write, this, std::placeholders::_1, std::placeholders::_2));
		this->write_in_progress_ = true;
		this->last_screen_       = std::move(screen);
	}

	void on_write(boost::system::error_code ec, std::size_t)
	{
		this->write_in_progress_ = false;
		if (ec)
		{
			BOOST_THROW_EXCEPTION(boost::system::system_error(ec, "write"));
		}
		if (this->next_screen_)
		{
			this->write_next_screen(std::move(this->next_screen_));
		}
	}

	std::string render_full(const Screen& screen)
	{
		this->terminal_output_.cursor(false);

		auto attr = boost::make_optional<tui_attribute::type>(false, {});
		int  y    = 0;
		for (const auto& row : screen.rows)
		{
			this->terminal_output_.movexy(0, y++);
			for (const auto& cell : row.cells)
			{
				if (cell.a != attr)
				{
					this->terminal_output_.attribute(cell.a);
					attr = cell.a;
				}
				this->terminal_output_.print(cell.c);
			}
		}

		if (this->cursor_)
		{
			this->terminal_output_.cursor(true);
		}

		return this->terminal_output_.unbuffer();
	}

	std::string render_diff(const Screen& screen, const Screen& previous)
	{
		this->terminal_output_.cursor(false);

		auto attr = boost::make_optional<tui_attribute::type>(false, {});
		for (size_t y = 0; y < screen.rows.size(); ++y)
		{
			const auto& row    = screen.rows[y];
			auto        render = false;
			if (y < previous.rows.size())
			{
				const auto& previousRow = previous.rows[y];
				if (row.cells.size() > previousRow.cells.size())
				{
					render = true;
				}
				else
				{
					render = !std::equal(row.cells.begin(), row.cells.end(), previousRow.cells.begin());
				}
			}
			else
			{
				render = true;
			}
			if (render)
			{
				this->terminal_output_.movexy(0, y);
				for (const auto& cell : row.cells)
				{
					if (cell.a != attr)
					{
						this->terminal_output_.attribute(cell.a);
						attr = cell.a;
					}
					this->terminal_output_.print(cell.c);
				}
			}
		}

		if (this->cursor_)
		{
			this->terminal_output_.cursor(true);
		}

		return this->terminal_output_.unbuffer();
	}

	void key_pressed(int key)
	{
		constexpr auto     SHIFT   = static_cast<int>(key_modifier::SHIFT);
		constexpr auto     CONTROL = static_cast<int>(key_modifier::CONTROL);
		constexpr auto     ALT     = static_cast<int>(key_modifier::ALT);
		constexpr auto     META    = static_cast<int>(key_modifier::META);
		fmt::memory_buffer buf{};
		if (key & SHIFT)
		{
			fmt::format_to(std::back_inserter(buf), "SHIFT ");
			key &= ~SHIFT;
		}
		if (key & CONTROL)
		{
			fmt::format_to(std::back_inserter(buf), "CONTROL ");
			key &= ~CONTROL;
		}
		if (key & ALT)
		{
			fmt::format_to(std::back_inserter(buf), "ALT ");
			key &= ~ALT;
		}
		if (key & META)
		{
			fmt::format_to(std::back_inserter(buf), "META ");
			key &= ~META;
		}
		if (std::isprint(key))
		{
			fmt::format_to(std::back_inserter(buf), "'{0:c}' 0x{0:02x}", key);
		}
		else
		{
			switch (static_cast<key_code>(key))
			{
			case key_code::BACKSPACE:
				fmt::format_to(std::back_inserter(buf), "BACKSPACE");
				break;
			case key_code::TAB:
				fmt::format_to(std::back_inserter(buf), "TAB");
				break;
			case key_code::ENTER:
				fmt::format_to(std::back_inserter(buf), "ENTER");
				break;
			case key_code::ESC:
				fmt::format_to(std::back_inserter(buf), "ESC");
				break;
			case key_code::UP:
				fmt::format_to(std::back_inserter(buf), "UP");
				break;
			case key_code::DOWN:
				fmt::format_to(std::back_inserter(buf), "DOWN");
				break;
			case key_code::RIGHT:
				fmt::format_to(std::back_inserter(buf), "RIGHT");
				break;
			case key_code::LEFT:
				fmt::format_to(std::back_inserter(buf), "LEFT");
				break;
			case key_code::INS:
				fmt::format_to(std::back_inserter(buf), "INS");
				break;
			case key_code::DEL:
				fmt::format_to(std::back_inserter(buf), "DEL");
				break;
			case key_code::PGUP:
				fmt::format_to(std::back_inserter(buf), "PGUP");
				break;
			case key_code::PGDOWN:
				fmt::format_to(std::back_inserter(buf), "PGDOWN");
				break;
			case key_code::HOME:
				fmt::format_to(std::back_inserter(buf), "HOME");
				break;
			case key_code::END:
				fmt::format_to(std::back_inserter(buf), "END");
				break;
			case key_code::F0:
				fmt::format_to(std::back_inserter(buf), "F0");
				break;
			case key_code::F1:
				fmt::format_to(std::back_inserter(buf), "F1");
				break;
			case key_code::F2:
				fmt::format_to(std::back_inserter(buf), "F2");
				break;
			case key_code::F3:
				fmt::format_to(std::back_inserter(buf), "F3");
				break;
			case key_code::F4:
				fmt::format_to(std::back_inserter(buf), "F4");
				break;
			case key_code::F5:
				fmt::format_to(std::back_inserter(buf), "F5");
				break;
			case key_code::F6:
				fmt::format_to(std::back_inserter(buf), "F6");
				break;
			case key_code::F7:
				fmt::format_to(std::back_inserter(buf), "F7");
				break;
			case key_code::F8:
				fmt::format_to(std::back_inserter(buf), "F8");
				break;
			case key_code::F9:
				fmt::format_to(std::back_inserter(buf), "F9");
				break;
			case key_code::F10:
				fmt::format_to(std::back_inserter(buf), "F10");
				break;
			case key_code::F11:
				fmt::format_to(std::back_inserter(buf), "F11");
				break;
			case key_code::F12:
				fmt::format_to(std::back_inserter(buf), "F12");
				break;
			case key_code::F13:
				fmt::format_to(std::back_inserter(buf), "F13");
				break;
			case key_code::F14:
				fmt::format_to(std::back_inserter(buf), "F14");
				break;
			case key_code::F15:
				fmt::format_to(std::back_inserter(buf), "F15");
				break;
			case key_code::F16:
				fmt::format_to(std::back_inserter(buf), "F16");
				break;
			case key_code::F17:
				fmt::format_to(std::back_inserter(buf), "F17");
				break;
			case key_code::F18:
				fmt::format_to(std::back_inserter(buf), "F18");
				break;
			case key_code::F19:
				fmt::format_to(std::back_inserter(buf), "F19");
				break;
			case key_code::F20:
				fmt::format_to(std::back_inserter(buf), "F20");
				break;
			default:
				fmt::format_to(std::back_inserter(buf), "{0:3d} 0x{0:02x}", key);
				break;
			}
		}
		ZOO_LOG(debug, "key: {}", fmt::to_string(buf));
	}
};

asio_terminal::asio_terminal(boost::asio::io_context& ioc)
    : pimpl_{ std::make_unique<impl>(ioc) }
{
}

asio_terminal::~asio_terminal() noexcept
{
}

void asio_terminal::set_key_pressed_handler(std::function<void(int)> handler)
{
	return this->pimpl_->set_key_pressed_handler(std::move(handler));
}

void asio_terminal::set_trap_ctrl_c(bool on)
{
	return this->pimpl_->set_trap_ctrl_c(on);
}

void asio_terminal::set_window_size_changed_handler(std::function<void()> handler)
{
	return this->pimpl_->set_window_size_changed_handler(std::move(handler));
}

tui_size asio_terminal::size() const
{
	return this->pimpl_->size();
}

void asio_terminal::cursor(bool on)
{
	return this->pimpl_->cursor(on);
}

void asio_terminal::cls(tui_attribute::type attr)
{
	return this->pimpl_->cls(attr);
}

void asio_terminal::print(tui_attribute::type attr, int x, int y, string_view s)
{
	return this->pimpl_->print(attr, x, y, std::move(s));
}

void asio_terminal::print(tui_attribute::type attr, int x, int y, u32_string_view s)
{
	return this->pimpl_->print(attr, x, y, std::move(s));
}

void asio_terminal::print(tui_attribute::type attr, int x, int y, char32_t c)
{
	return this->pimpl_->print(attr, x, y, c);
}

void asio_terminal::print(tui_attribute::type attr, const tui_position& pos, string_view s)
{
	return this->pimpl_->print(attr, pos.x, pos.y, std::move(s));
}

void asio_terminal::print(tui_attribute::type attr, const tui_position& pos, u32_string_view s)
{
	return this->pimpl_->print(attr, pos.x, pos.y, std::move(s));
}

void asio_terminal::print(tui_attribute::type attr, const tui_position& pos, char32_t c)
{
	return this->pimpl_->print(attr, pos.x, pos.y, c);
}

void asio_terminal::horizontal_line(tui_attribute::type attr, const tui_position& pos, int width)
{
	return this->pimpl_->horizontal_line(attr, pos, width, character::GC_HLINE);
}

void asio_terminal::horizontal_line(tui_attribute::type attr, const tui_position& pos, int width, char32_t c)
{
	return this->pimpl_->horizontal_line(attr, pos, width, c);
}

void asio_terminal::vertical_line(tui_attribute::type attr, const tui_position& pos, int height)
{
	return this->pimpl_->vertical_line(attr, pos, height, character::GC_VLINE);
}

void asio_terminal::vertical_line(tui_attribute::type attr, const tui_position& pos, int height, char32_t c)
{
	return this->pimpl_->vertical_line(attr, pos, height, c);
}

void asio_terminal::update()
{
	return this->pimpl_->update();
}

} // namespace tui
} // namespace zoo
