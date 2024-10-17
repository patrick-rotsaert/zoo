#pragma once

namespace zoo {
namespace tui {

enum class key_code : int
{
	BACKSPACE = 8,
	TAB       = '\t',
	ENTER     = '\r',
	ESC       = 27,
	SPACE     = ' ',
	KEYPAD_5  = '5',
	UP        = (0x100 | 72),
	DOWN      = (0x100 | 80),
	RIGHT     = (0x100 | 77),
	LEFT      = (0x100 | 75),
	INS       = (0x100 | 82),
	DEL       = (0x100 | 83),
	PGUP      = (0x100 | 73),
	PGDOWN    = (0x100 | 81),
	HOME      = (0x100 | 71),
	END       = (0x100 | 79),
	F0        = (0x100 | 58),
	F1        = (0x100 | 59),
	F2        = (0x100 | 60),
	F3        = (0x100 | 61),
	F4        = (0x100 | 62),
	F5        = (0x100 | 63),
	F6        = (0x100 | 64),
	F7        = (0x100 | 65),
	F8        = (0x100 | 66),
	F9        = (0x100 | 67),
	F10       = (0x100 | 68),
	F11       = (0x100 | 133),
	F12       = (0x100 | 134),
	F13       = (0x100 | 135),
	F14       = (0x100 | 136),
	F15       = (0x100 | 137),
	F16       = (0x100 | 138),
	F17       = (0x100 | 139),
	F18       = (0x100 | 140),
	F19       = (0x100 | 141),
	F20       = (0x100 | 142),
};

}
} // namespace zoo
