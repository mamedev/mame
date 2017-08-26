// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    PSI HLE Keyboard

***************************************************************************/

#include "emu.h"
#include "hle.h"
#include "machine/keyboard.h"
#include "machine/keyboard.ipp"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSI_HLE_KEYBOARD, psi_hle_keyboard_device, "psi_hle_kbd", "PSI HLE Keyboard")

namespace {

uint8_t const TRANSLATION_TABLE[4][7][16] =
{
	// unshift
	{
		{ 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x00, 0x00, 0x00 }, // 0
		{ 0x1b, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x7e, 0x60, 0x7c, 0x7f, 0x00 }, // 1
		{ 0x09, 0x71, 0x77, 0x65, 0x72, 0x74, 0x7a, 0x75, 0x69, 0x6f, 0x70, 0x7d, 0x2b, 0x0a, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x61, 0x73, 0x64, 0x66, 0x67, 0x68, 0x6a, 0x6b, 0x6c, 0x7c, 0x7b, 0x23, 0x0d, 0x00 }, // 3
		{ 0x00, 0x3c, 0x79, 0x78, 0x63, 0x76, 0x62, 0x6e, 0x6d, 0x2c, 0x2e, 0x2d, 0x00, 0x20, 0x00, 0x00 }, // 4
		{ 0xef, 0x7e, 0x40, 0xff, 0x5b, 0x5d, 0x5c, 0xa1, 0xa2, 0xa4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 5
		{ 0xa5, 0x1a, 0xa6, 0x08, 0x1c, 0x06, 0xa7, 0x0a, 0xa8, 0xa9, 0xbd, 0xbe, 0x00, 0x00, 0x00, 0x00 }  // 6
	},
	// shift
	{
		{ 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x00, 0x00, 0x00 }, // 0
		{ 0x1b, 0x21, 0x22, 0x40, 0x24, 0x25, 0x26, 0x2f, 0x28, 0x29, 0x3d, 0x3f, 0x60, 0x5e, 0x7f, 0x00 }, // 1
		{ 0x09, 0x51, 0x57, 0x45, 0x52, 0x54, 0x5a, 0x55, 0x49, 0x4f, 0x50, 0x5d, 0x2a, 0x0a, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x5c, 0x5b, 0x27, 0x0d, 0x00 }, // 3
		{ 0x00, 0x3e, 0x59, 0x58, 0x43, 0x56, 0x42, 0x4e, 0x4d, 0x3b, 0x3a, 0x5f, 0x00, 0x20, 0x00, 0x00 }, // 4
		{ 0xef, 0x7e, 0x40, 0xff, 0x7b, 0x7d, 0x7c, 0xa1, 0xa2, 0xa4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 5
		{ 0xa5, 0x1a, 0xa6, 0x08, 0x1c, 0x06, 0xa7, 0x0a, 0xa8, 0xa9, 0xbd, 0xbe, 0x00, 0x00, 0x00, 0x00 }  // 6
	},
	// unshift-control
	{
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0 (should output control codes here)
		{ 0x00, 0x8d, 0x27, 0x93, 0x92, 0x86, 0x91, 0x84, 0x94, 0x95, 0x98, 0x99, 0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x11, 0x17, 0x05, 0x12, 0x14, 0x1a, 0x15, 0x09, 0x0f, 0x10, 0x00, 0x9c, 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x01, 0x13, 0x04, 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x8b, 0x85, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x8f, 0x19, 0x18, 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 5
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 6
	},
	// shift-control
	{
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x81, 0x27, 0x82, 0x83, 0x8c, 0x9f, 0x9e, 0x96, 0x97, 0x9a, 0x9b, 0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x11, 0x17, 0x05, 0x12, 0x14, 0x1a, 0x15, 0x09, 0x0f, 0x10, 0x00, 0x9d, 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x01, 0x13, 0x04, 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0c, 0x89, 0x90, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x8e, 0x19, 0x18, 0x03, 0x16, 0x02, 0x0e, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 5
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 6
	}
};

} // anonymous namespace

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( keyboard )
	PORT_START("mod")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CTRL")       PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)             PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT")      PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)               PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAPS")       PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("row_0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)                                PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)                                PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)                                PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)                                PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)                                PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)                                PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)                                PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)                                PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)                                PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)                               PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)                               PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)                               PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR)     PORT_NAME("F13")       PORT_CHAR(UCHAR_MAMEKEY(F13))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED) // F
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED) // CAPS
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("ESC/STOP")  PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2')  PORT_CHAR('"')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)                                 PORT_CHAR('3')  PORT_CHAR(0xa7) // §
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)                                 PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)                                 PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)                                 PORT_CHAR('6')  PORT_CHAR('&')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)                                 PORT_CHAR('7')  PORT_CHAR('/')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)                                 PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)                                 PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)                                 PORT_CHAR('0')  PORT_CHAR('=')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)                             PORT_CHAR(0xdf) PORT_CHAR('?') // ß
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)                            PORT_CHAR(0xb4) PORT_CHAR('`')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)                             PORT_CHAR('|')  PORT_CHAR('^')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("RUB OUT")   PORT_CHAR(0x08)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)                               PORT_CHAR('\t')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)                                 PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)                                 PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)                                 PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)                                 PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)                                 PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)                                 PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)                                 PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)                                 PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)                                 PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)                         PORT_CHAR(0xfc) PORT_CHAR(0xdc) // ü Ü
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR('+')  PORT_CHAR('*')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED) // LF, same code as cursor down
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED) // CTRL
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_UNUSED) // LOCK
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)                                 PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)                                 PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)                                 PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)                                 PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)                                 PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)                                 PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)                                 PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)                                 PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)                                 PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)                             PORT_CHAR(0xf6) PORT_CHAR(0xd6) // ö Ö
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(0xe4) PORT_CHAR(0xc4) // ä Ä
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('#')  PORT_CHAR('\'')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)         PORT_NAME("CR")     PORT_CHAR(0x0d)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED) // LSHIFT
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR('<')  PORT_CHAR('>')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)                                 PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)                                 PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)                                 PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)                                 PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)                                 PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)                                 PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)                                 PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)                             PORT_CHAR(',')  PORT_CHAR(';')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)                              PORT_CHAR('.')  PORT_CHAR(':')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED) // RSHIFT
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_NUMLOCK)   PORT_NAME("DIN")         PORT_TOGGLE
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)     PORT_NAME("~")           PORT_CHAR('~')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)   PORT_NAME("@")           PORT_CHAR('@')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)     PORT_NAME("BREAK")       PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("[ {")         PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)  PORT_NAME("] }")         PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("\\ |")        PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)       PORT_NAME("DEL CHAR")    PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)       PORT_NAME("ER \xe2\x87\xa5") // ER ⇥
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_NAME("ER PAGE")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)                            PORT_NAME("\xe2\x86\x96") // ↖
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_UP)      PORT_NAME("\xe2\x86\x91") PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)                            PORT_NAME("\xe2\x86\x97") // ↗
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_LEFT)    PORT_NAME("\xe2\x86\x90") PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)                             PORT_NAME("HOME")         PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_RIGHT)   PORT_NAME("\xe2\x86\x92") PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)                            PORT_NAME("\xe2\x86\x99") // ↙
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_DOWN)    PORT_NAME("\xe2\x86\x93") PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)                            PORT_NAME("\xe2\x86\x98") // ↘
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)                             PORT_NAME("\xe2\x87\xa7")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)                           PORT_NAME("INS CHAR")     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)                             PORT_NAME("\xe2\x87\xa9")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor psi_hle_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psi_hle_keyboard_device - constructor
//-------------------------------------------------

psi_hle_keyboard_device::psi_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PSI_HLE_KEYBOARD, tag, owner, clock),
	device_psi_keyboard_interface(mconfig, *this),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4", "row_5", "row_6"),
	m_modifiers(*this, "mod")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psi_hle_keyboard_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psi_hle_keyboard_device::device_reset()
{
	reset_key_state();
	start_processing(attotime::from_hz(9600));
	typematic_stop();
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void psi_hle_keyboard_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = translate(row, column);

	if (code != 0x00)
	{
		send_key(code);
		typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));
	}
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void psi_hle_keyboard_device::key_break(uint8_t row, uint8_t column)
{
	if (typematic_is(row, column))
		typematic_stop();

	uint8_t code = translate(row, column);

	// special handling for DIN key
	if (code == 0xef)
		send_key(0xee);
}

//-------------------------------------------------
//  key_repeat - handle a key being repeated
//-------------------------------------------------

void psi_hle_keyboard_device::key_repeat(u8 row, u8 column)
{
	uint8_t code = translate(row, column);
	send_key(code);
}

//-------------------------------------------------
//  translate - row and column to key code
//-------------------------------------------------

uint8_t psi_hle_keyboard_device::translate(uint8_t row, uint8_t column)
{
	uint8_t const modifiers(m_modifiers->read());

	bool const ctrl(modifiers & 0x01);
	bool const shift(bool(modifiers & 0x02) || (bool(modifiers & 0x04)));
	bool const ctrl_shift(ctrl && shift);

	unsigned const map(ctrl_shift? 3 : ctrl ? 2 : shift ? 1 : 0);

	return TRANSLATION_TABLE[map][row][column];
}

//-------------------------------------------------
//  send_key - send key code to host
//-------------------------------------------------

void psi_hle_keyboard_device::send_key(uint8_t code)
{
	m_host->key_data_w(machine().dummy_space(), 0, code);
	m_host->key_strobe_w(1);
	m_host->key_strobe_w(0);
}
