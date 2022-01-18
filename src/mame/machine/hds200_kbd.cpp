// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    HDS200 Keyboard (HLE)

    TODO:
    - Add missing keycodes, verify existing
    - LEDs
    - Keyclick

    Notes:
    - No clear picture of the keyboard available, everything guessed

***************************************************************************/

#include "emu.h"
#include "hds200_kbd.h"
#include "machine/keyboard.ipp"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(HDS200_KBD_HLE, hds200_kbd_hle_device, "hds200_kbd_hle", "HDS200 Keyboard (HLE)")

namespace {

uint8_t const TRANSLATION_TABLE[4][7][22] =
{
	// unshift
	{ //  main                                                                                         control              number pad
		{ 0xaf, 0xb0, 0x00, 0xb1, 0xb2, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x00,    0x8c, 0x8d, 0x8e,    0x93, 0x94, 0x95, 0x96 }, // 0
		{ 0xb3,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  '`', 0xb5,    0x97, 0x98, 0x99,    0x8f, 0x90, 0x91, 0x92 }, // 1
		{ 0xb4,  'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', 0x0d, 0xb6,    0xac, 0xad, 0xae,    0xa5, 0xa6, 0xa7, 0x9c }, // 2
		{ 0x00,  'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'', 0x00, 0x00, '\\',    0x00, 0xa8, 0x00,    0xa2, 0xa3, 0xa4, 0x9b }, // 3
		{ 0x00, 0x00,  'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '/', 0x00, 0x00, 0xb7,    0xab, 0xa9, 0xaa,    0x9f, 0xa0, 0xa1, 0x9a }, // 4
		{ 0x00,  ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb9,    0x00, 0x00, 0x00,    0x9e, 0x00, 0x9d, 0x00 }  // 5
	},
	// shift
	{ //  main                                                                                         control              number pad
		{ 0xef, 0xf0, 0x00, 0xf1, 0xf2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0x00,    0xcc, 0xcd, 0xce,    0xd3, 0xd4, 0xd5, 0xd6 }, // 0
		{ 0xf3,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+',  '~', 0xf5,    0xd7, 0xd8, 0xd9,    0xcf, 0xd0, 0xd1, 0xd2 }, // 1
		{ 0xf4,  'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', 0x0d, 0xf6,    0xec, 0xed, 0xee,    0xe5, 0xe6, 0xe7, 0xdc }, // 2
		{ 0x00,  'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"', 0x00, 0x00,  '|',    0x00, 0xe8, 0x00,    0xe2, 0xe3, 0xe4, 0xdb }, // 3
		{ 0x00, 0x00,  'Z',  'X',  'C',  'V',  'B',  'N',  'M',  '<',  '>',  '?', 0x00, 0x00, 0xf7,    0xeb, 0xe9, 0xea,    0xdf, 0xe0, 0xe1, 0xda }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf9,    0x00, 0x00, 0x00,    0xde, 0x00, 0xdd, 0x00 }  // 5
	},
	// unshift-control
	{ //  main                                                                                         control              number pad
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }  // 5
	},
	// shift-control
	{ //  main                                                                                         control              number pad
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    0x00, 0x00, 0x00,    0x00, 0x00, 0x00, 0x00 }  // 5
	}
};

} // anonymous namespace

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( keyboard )
	PORT_START("mod")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl")        PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Shift")  PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("row_0")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SCRLOCK) PORT_NAME("No Scroll")
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)  PORT_NAME("Setup  Status")
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_UNUSED) // double width key
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)     PORT_NAME("Halt  Reset")
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)   PORT_NAME("Break  Long Break")
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)      PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)      PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)      PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)      PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)      PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)      PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)      PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)      PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)      PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_UNUSED) // one key less in this row
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)     PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)     PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)     PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F17)     PORT_CHAR(UCHAR_MAMEKEY(F17))
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F18)     PORT_CHAR(UCHAR_MAMEKEY(F18))
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F19)     PORT_CHAR(UCHAR_MAMEKEY(F19))
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F20)     PORT_CHAR(UCHAR_MAMEKEY(F20))
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_1")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)       PORT_CHAR(27)  PORT_NAME("Esc  CMD")
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F21") // PORT_CODE(KEYCODE_F21) (not supported by core)
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F22") // PORT_CODE(KEYCODE_F22) (not supported by core)
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F23") // PORT_CODE(KEYCODE_F23) (not supported by core)
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F13)       PORT_CHAR(UCHAR_MAMEKEY(F13)) PORT_NAME("F13 (PF1)") 
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F14)       PORT_CHAR(UCHAR_MAMEKEY(F14)) PORT_NAME("F14 (PF2)") 
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F15)       PORT_CHAR(UCHAR_MAMEKEY(F15)) PORT_NAME("F15 (PF3)") 
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F16)       PORT_CHAR(UCHAR_MAMEKEY(F16)) PORT_NAME("F16 (PF4)") 
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_2")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_NAME("Scroll Down  Scroll Up")
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_NAME("Page Down  Page Up")     
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_3")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_UNUSED) // Ctrl (handled elsewhere)
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)         PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD)) PORT_NAME("Keypad ,")
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_4")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_UNUSED) // Left Shift (handled elsewhere)
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)         PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_UNUSED) // Right Shift (handled elsewhere)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Line Feed")
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_5")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Comp Char")
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)    PORT_CHAR(' ')
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("Caps Lock")
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)    PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x080000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x100000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)  PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x200000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x400000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x800000, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor hds200_kbd_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}

void hds200_kbd_hle_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_bell, 786).add_route(ALL_OUTPUTS, "mono", 0.5); // unknown frequency
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hds200_kbd_hle_device - constructor
//-------------------------------------------------

hds200_kbd_hle_device::hds200_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HDS200_KBD_HLE, tag, owner, clock),
	device_buffered_serial_interface(mconfig, *this),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4", "row_5"),
	m_bell(*this, "bell"),
	m_modifiers(*this, "mod"),
	m_tx_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hds200_kbd_hle_device::device_start()
{
	// resolve callbacks
	m_tx_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hds200_kbd_hle_device::device_reset()
{
	clear_fifo();

	receive_register_reset();
	transmit_register_reset();

	set_data_frame(1, 8, PARITY_EVEN, STOP_BITS_2);
	set_rcv_rate(2400);
	set_tra_rate(2400);

	reset_key_state();
	start_processing(attotime::from_hz(2400));
	typematic_stop();
}

//-------------------------------------------------
//  tra_callback - send bit to host
//-------------------------------------------------

void hds200_kbd_hle_device::tra_callback()
{
	m_tx_handler(transmit_register_get_data_bit());
}

//-------------------------------------------------
//  received_byte - handle received byte
//-------------------------------------------------

void hds200_kbd_hle_device::received_byte(uint8_t byte)
{
	logerror("Received from host: %02x\n", byte);

	switch (byte & 0xc0)
	{
		// state
		case 0x00:

			// 76------  00
			// --5-----  caps lock led
			// ---43---  unknown
			// -----2--  insert mode
			// ------1-  led l1
			// -------0  bell

			m_bell->set_state(BIT(byte, 0));
			break;

		// volume
		case 0xc0:

			// 76------  11
			// --543---  bell volume
			// -----210  keyclick volume

			int bell_vol = (byte >> 3) & 0x07;
			m_bell->set_output_gain(0, bell_vol / 7.0);
			break;
	}
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void hds200_kbd_hle_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = translate(row, column);

	// send the code
	if (code != 0x00)
	{
		transmit_byte(code);
		typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));
	}
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void hds200_kbd_hle_device::key_break(uint8_t row, uint8_t column)
{
	if (typematic_is(row, column))
		typematic_stop();
}

//-------------------------------------------------
//  key_repeat - handle a key being repeated
//-------------------------------------------------

void hds200_kbd_hle_device::key_repeat(u8 row, u8 column)
{
	uint8_t code = translate(row, column);

	if (code != 0x00)
		transmit_byte(code);
}

//-------------------------------------------------
//  rx_w - receive bit from host
//-------------------------------------------------

void hds200_kbd_hle_device::rx_w(int state)
{
	device_buffered_serial_interface::rx_w(state);
}

//-------------------------------------------------
//  translate - row and column to key code
//-------------------------------------------------

uint8_t hds200_kbd_hle_device::translate(uint8_t row, uint8_t column)
{
	uint8_t const modifiers(m_modifiers->read());

	bool const ctrl(modifiers & 0x01);
	bool const shift(bool(modifiers & 0x02) || bool(modifiers & 0x04));
	bool const ctrl_shift(ctrl && shift);

	unsigned const map(ctrl_shift? 3 : ctrl ? 2 : shift ? 1 : 0);

	return TRANSLATION_TABLE[map][row][column];
}
