#pragma once

#include "zoo/tui/config.h"

namespace zoo {
namespace tui {

// Graphic characters (values are as in DOS char set)
struct ZOO_TUI_API character
{
	static constexpr auto GC_UARROW   = U'↑'; // arrow pointing up
	static constexpr auto GC_DARROW   = U'↓'; // arrow pointing down
	static constexpr auto GC_RARROW   = U'→'; // arrow pointing right
	static constexpr auto GC_LARROW   = U'←'; // arrow pointing left
	static constexpr auto GC_BLOCK    = U'█'; // solid square block
	static constexpr auto GC_CKBOARD  = U'░'; // checker board (stipple)
	static constexpr auto GC_PLMINUS  = U'±'; // plus/minus
	static constexpr auto GC_BOARD    = U'░'; // board of squares
	static constexpr auto GC_LRCORNER = U'┘'; // lower right corner
	static constexpr auto GC_URCORNER = U'┐'; // upper right corner
	static constexpr auto GC_ULCORNER = U'┌'; // upper left corner
	static constexpr auto GC_LLCORNER = U'└'; // lower left corner
	static constexpr auto GC_PLUS     = U'┼'; // large plus or crossover
	static constexpr auto GC_S1       = U'■'; // scan line 1
	static constexpr auto GC_HLINE    = U'─'; // horizontal line
	static constexpr auto GC_LTEE     = U'├'; // tee pointing right
	static constexpr auto GC_RTEE     = U'┤'; // tee pointing left
	static constexpr auto GC_BTEE     = U'┴'; // tee pointing up
	static constexpr auto GC_TTEE     = U'┬'; // tee pointing down
	static constexpr auto GC_VLINE    = U'│'; // vertical line
	static constexpr auto GC_BULLET   = U'·'; // bullet
	static constexpr auto GC_LEQUAL   = U'≤'; // less-than-or-equal-to
	static constexpr auto GC_GEQUAL   = U'≥'; // greater-than-or-equal-to
	static constexpr auto GC_DIAMOND  = U'♦'; // diamond
	static constexpr auto GC_STERLING = U'£'; // UK pound sign
	static constexpr auto GC_DEGREE   = U'°'; // degree symbol
	static constexpr auto GC_PI       = U'π'; // greek pi
};

} // namespace tui
} // namespace zoo
