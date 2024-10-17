#pragma once

#include "zoo/tui/config.h"

#include <cstdint>

namespace zoo {
namespace tui {

struct ZOO_TUI_API tui_attribute
{
	using type = std::uint8_t;

	static constexpr type FG_BLACK = 0x00;

	static constexpr type FG_BLUE  = 0x01;
	static constexpr type FG_GREEN = 0x02;
	static constexpr type FG_RED   = 0x04;

	static constexpr type FG_CYAN      = (FG_BLUE | FG_GREEN);
	static constexpr type FG_MAGENTA   = (FG_BLUE | FG_RED);
	static constexpr type FG_BROWN     = (FG_GREEN | FG_RED);
	static constexpr type FG_LIGHTGRAY = (FG_BLUE | FG_GREEN | FG_RED);

	static constexpr type FG_BRIGHT = 0x08;
	static constexpr type FG_BLINK  = 0x80;

	static constexpr type FG_DARKGRAY     = (FG_BRIGHT | FG_BLACK);
	static constexpr type FG_LIGHTBLUE    = (FG_BRIGHT | FG_BLUE);
	static constexpr type FG_LIGHTGREEN   = (FG_BRIGHT | FG_GREEN);
	static constexpr type FG_LIGHTCYAN    = (FG_BRIGHT | FG_CYAN);
	static constexpr type FG_LIGHTRED     = (FG_BRIGHT | FG_RED);
	static constexpr type FG_LIGHTMAGENTA = (FG_BRIGHT | FG_MAGENTA);
	static constexpr type FG_YELLOW       = (FG_BRIGHT | FG_BROWN);
	static constexpr type FG_WHITE        = (FG_BRIGHT | FG_LIGHTGRAY);

	static constexpr type BG_BLACK = 0x00;

	static constexpr type BG_BLUE  = 0x10;
	static constexpr type BG_GREEN = 0x20;
	static constexpr type BG_RED   = 0x40;

	static constexpr type BG_CYAN      = (BG_BLUE | BG_GREEN);
	static constexpr type BG_MAGENTA   = (BG_BLUE | BG_RED);
	static constexpr type BG_BROWN     = (BG_GREEN | BG_RED);
	static constexpr type BG_LIGHTGRAY = (BG_BLUE | BG_GREEN | BG_RED);

	static constexpr type BG_MASK = 0x70;
	static constexpr type FG_MASK = 0x0F;
};

} // namespace tui
} // namespace zoo
