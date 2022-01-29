// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Regnecentralen RC759 Piccoline Keyboard (HLE)

***************************************************************************/

#include "emu.h"
#include "rc759_kbd.h"
#include "machine/keyboard.ipp"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(RC759_KBD_HLE, rc759_kbd_hle_device, "rc759_kbd_hle", "RC759 Keyboard (HLE)")


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( keyboard )
	PORT_START("row_0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_UNUSED)   /* 00 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 01 */ PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 02 */ PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 03 */ PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 04 */ PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR(0xa7) // §
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 05 */ PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 06 */ PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 07 */ PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 08 */ PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'') // or ´?
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 09 */ PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 10 */ PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 11 */ PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR('-')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 12 */ PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('_') PORT_CHAR('=')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 13 */ PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('`') PORT_CHAR('@')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 14 */ PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 15 */ PORT_CODE(KEYCODE_LALT)       PORT_NAME("Alt")

	PORT_START("row_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 16 */ PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 17 */ PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 18 */ PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 19 */ PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 20 */ PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 21 */ PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 22 */ PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 23 */ PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 24 */ PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 25 */ PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 26 */ PORT_CODE(KEYCODE_COLON)      PORT_CHAR(0xe6) PORT_CHAR(0xc6) // æ Æ
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 27 */ PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(0xe5) PORT_CHAR(0xc5) // å Å
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 28 */ PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 29 */ PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 30 */ PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 31 */ PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("row_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 32 */ PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 33 */ PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 34 */ PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 35 */ PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 36 */ PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 37 */ PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 38 */ PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 39 */ PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 40 */ PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 41 */ PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 42 */ PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Left Shift")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 43 */ PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(0xf8) PORT_CHAR(0xd8) // ø Ø
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 44 */ PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 45 */ PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 46 */ PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 47 */ PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("row_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 48 */ PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 49 */ PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 50 */ PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 51 */ PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 52 */ PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 53 */ PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 54 */ PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Right Shift")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 55 */ PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 56 */ PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(0xfc) PORT_CHAR(0xdc) // ü Ü
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 57 */ PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 58 */ PORT_CODE(KEYCODE_PRTSCR) PORT_NAME("Print")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 59 */ PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 60 */ PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 61 */ PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 62 */ PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 63 */ PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("row_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 64 */ PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 65 */ PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 66 */ PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 67 */ PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 68 */ PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 69 */ PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 70 */ PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 71 */ PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_NAME("Tegn Ind")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 72 */ PORT_CODE(KEYCODE_PGUP)       PORT_NAME("A1")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 73 */ PORT_CODE(KEYCODE_PGDN)       PORT_NAME("A2")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 74 */ PORT_CODE(KEYCODE_END)        PORT_NAME("A3")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 75 */ PORT_CODE(KEYCODE_NUMLOCK)    PORT_NAME("A4")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 76 */ PORT_CODE(KEYCODE_SCRLOCK)    PORT_NAME("(((O)))") // Bell?
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 77 */ PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 78 */ PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 79 */ PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("row_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 80 */ PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 81 */ PORT_CODE(KEYCODE_HOME)      PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_NAME("\xe2\x86\x96") // ↖
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 82 */ PORT_CODE(KEYCODE_DEL)       PORT_CHAR(UCHAR_MAMEKEY(DEL))  PORT_NAME("Slet Tegn")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 83 */ PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 84 */ PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 85 */ PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 86 */ PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 87 */ PORT_CODE(KEYCODE_PLUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 88 */ PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 89 */ PORT_CODE(KEYCODE_5_PAD)     PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 90 */ PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 91 */ PORT_CODE(KEYCODE_ASTERISK)  PORT_NAME("Keypad ,")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 92 */ PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("Keypad Tab")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 93 */ PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 94 */ PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 95 */ PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	PORT_START("row_6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 96 */ PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 97 */ PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 98 */ PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
INPUT_PORTS_END

ioport_constructor rc759_kbd_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rc759_kbd_hle_device - constructor
//-------------------------------------------------

rc759_kbd_hle_device::rc759_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, RC759_KBD_HLE, tag, owner, clock),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4", "row_5", "row_6"),
	m_int_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rc759_kbd_hle_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();

	// register for save states
	save_item(NAME(m_data));
	save_item(NAME(m_enabled));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rc759_kbd_hle_device::device_reset()
{
	m_int_handler(0);
	m_data = 0x00;
	m_enabled = false;

	reset_key_state();
	start_processing(attotime::from_hz(1200));
	typematic_stop();
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void rc759_kbd_hle_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = row * 16 + column;

	// send the code
	if (m_enabled)
	{
		m_data = code;
		m_int_handler(1);
	}

	// no typematic for modifier keys
	if (code != 15 && code != 29 && code != 40 && code != 42 && code != 54)
		typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void rc759_kbd_hle_device::key_break(uint8_t row, uint8_t column)
{
	uint8_t code = row * 16 + column;

	// send the break code
	if (m_enabled)
	{
		m_data = 0x80 | code;
		m_int_handler(1);
	}

	if (typematic_is(row, column))
		typematic_stop();
}

//-------------------------------------------------
//  key_repeat - handle a key being repeated
//-------------------------------------------------

void rc759_kbd_hle_device::key_repeat(u8 row, u8 column)
{
	uint8_t code = row * 16 + column;

	if (m_enabled)
	{
		m_data = code;
		m_int_handler(1);
	}
}

//-------------------------------------------------
//  read - return current key data
//-------------------------------------------------

uint8_t rc759_kbd_hle_device::read()
{
	m_int_handler(0);
	return m_data;
}

//-------------------------------------------------
//  enable_w - enable or disable keyboard
//-------------------------------------------------

void rc759_kbd_hle_device::enable_w(int state)
{
	m_enabled = bool(state);
}
