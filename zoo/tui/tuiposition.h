#pragma once

#include "zoo/tui/config.h"

namespace zoo {
namespace tui {

struct ZOO_TUI_API tui_position
{
	int x, y;

	tui_position  operator+(const tui_position& other) const noexcept;
	tui_position& operator+=(const tui_position& other) noexcept;
};

} // namespace tui
} // namespace zoo
