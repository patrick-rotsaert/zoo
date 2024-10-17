#include "zoo/tui/tuisize.h"

namespace zoo {
namespace tui {

bool tui_size::operator<=(const tui_size& other) const noexcept
{
	return this->rows <= other.rows && this->columns <= other.columns;
}

} // namespace tui
} // namespace zoo
