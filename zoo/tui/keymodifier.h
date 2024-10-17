#pragma once

namespace zoo {
namespace tui {

enum class key_modifier : int
{
	SHIFT   = 0x1000,
	ALT     = 0x2000,
	CONTROL = 0x4000,
	META    = 0x8000
};

}
} // namespace zoo
