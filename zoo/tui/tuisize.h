#pragma once

#include "zoo/tui/config.h"

namespace zoo {
namespace tui {

struct ZOO_TUI_API tui_size
{
	int rows;
	int columns;

	bool operator<=(const tui_size& other) const noexcept;
};

} // namespace tui
} // namespace zoo
