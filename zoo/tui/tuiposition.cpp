#include "zoo/tui/tuiposition.h"

namespace zoo {
namespace tui {

tui_position tui_position::operator+(const tui_position& other) const noexcept
{
	return tui_position{ this->x + other.x, this->y + other.y };
}

tui_position& tui_position::operator+=(const tui_position& other) noexcept
{
	this->x += other.x;
	this->y += other.y;
	return *this;
}

} // namespace tui
} // namespace zoo
