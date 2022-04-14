// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Falco F5220 Keyboard

***************************************************************************/

#include "emu.h"
#include "f5220_kbd.h"
#include "machine/keyboard.ipp"


DEFINE_DEVICE_TYPE(F5220_KBD, f5220_kbd_device, "f5220_kbd", "F5220 Keyboard")


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

// Missing: Local Print, Break, Function

static INPUT_PORTS_START( keyboard )
	PORT_START("row_0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 00 */ PORT_NAME("00")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 01 */ PORT_CODE(KEYCODE_SCRLOCK)  PORT_NAME("Hold Session")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 02 */ PORT_NAME("02") // print?
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 03 */ PORT_NAME("03") // print?
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 04 */ PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("SetUp")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 05 */ PORT_NAME("05")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 06 */ PORT_NAME("06")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 07 */ PORT_NAME("Switch Session")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 08 */ PORT_CODE(KEYCODE_F6)       PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 09 */ PORT_CODE(KEYCODE_F7)       PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0a */ PORT_CODE(KEYCODE_F8)       PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0b */ PORT_CODE(KEYCODE_F9)       PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0c */ PORT_CODE(KEYCODE_F10)      PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0d */ PORT_CODE(KEYCODE_F11)      PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0e */ PORT_CODE(KEYCODE_F12)      PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0f */ PORT_CODE(KEYCODE_F13)      PORT_CHAR(UCHAR_MAMEKEY(F13))

	PORT_START("row_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 10 */ PORT_CODE(KEYCODE_F14)   PORT_CHAR(UCHAR_MAMEKEY(F14))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 11 */ PORT_CODE(KEYCODE_F15)   PORT_NAME("Help")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 12 */ PORT_CODE(KEYCODE_END)   PORT_NAME("Prev Scrn")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 13 */ PORT_CODE(KEYCODE_F16)   PORT_NAME("Do")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 14 */ PORT_NAME("14")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 15 */ PORT_CODE(KEYCODE_F17)   PORT_CHAR(UCHAR_MAMEKEY(F17))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 16 */ PORT_CODE(KEYCODE_F18)   PORT_CHAR(UCHAR_MAMEKEY(F18))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 17 */ PORT_CODE(KEYCODE_F19)   PORT_CHAR(UCHAR_MAMEKEY(F19))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 18 */ PORT_CODE(KEYCODE_F20)   PORT_CHAR(UCHAR_MAMEKEY(F20))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 19 */ PORT_CODE(KEYCODE_PGDN)  PORT_NAME("Next Scrn")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1a */ PORT_CODE(KEYCODE_TILDE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1b */ PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1c */ PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1d */ PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1e */ PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1f */ PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("row_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 20 */ PORT_CODE(KEYCODE_6)        PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 21 */ PORT_CODE(KEYCODE_7)        PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 22 */ PORT_CODE(KEYCODE_8)        PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 23 */ PORT_CODE(KEYCODE_9)        PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 24 */ PORT_CODE(KEYCODE_0)        PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 25 */ PORT_CODE(KEYCODE_MINUS)    PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 26 */ PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 27 */ PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 28 */ PORT_CODE(KEYCODE_INSERT)                                 PORT_NAME("Find")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 29 */ PORT_CODE(KEYCODE_HOME)                                   PORT_NAME("Insert Here")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2a */ PORT_CODE(KEYCODE_PGUP)                                   PORT_NAME("Remove")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2b */ PORT_CODE(KEYCODE_F2)       PORT_CHAR(UCHAR_MAMEKEY(F2))  PORT_NAME("PF2")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2c */ PORT_CODE(KEYCODE_F3)       PORT_CHAR(UCHAR_MAMEKEY(F3))  PORT_NAME("PF3")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2d */ PORT_CODE(KEYCODE_F4)       PORT_CHAR(UCHAR_MAMEKEY(F4))  PORT_NAME("PF4")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2e */ PORT_CODE(KEYCODE_ESC)      PORT_CHAR(27)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2f */ PORT_CODE(KEYCODE_TAB)      PORT_CHAR(9)

	PORT_START("row_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 30 */ PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 31 */ PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 32 */ PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 33 */ PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 34 */ PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 35 */ PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 36 */ PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 37 */ PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 38 */ PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 39 */ PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3a */ PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3b */ PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3c */ PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3d */ PORT_NAME("3d")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3e */ PORT_CODE(KEYCODE_DEL)        PORT_NAME("Select")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3f */ PORT_NAME("3f")

	PORT_START("row_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 40 */ PORT_CODE(KEYCODE_8_PAD)     PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 41 */ PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 42 */ PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 43 */ PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 44 */ PORT_CODE(KEYCODE_CAPSLOCK)  PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 45 */ PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 46 */ PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 47 */ PORT_CODE(KEYCODE_D)         PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 48 */ PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 49 */ PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4a */ PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4b */ PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4c */ PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4d */ PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4e */ PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4f */ PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('"')

	PORT_START("row_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 50 */ PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 51 */ PORT_NAME("51")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 52 */ PORT_NAME("52")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 53 */ PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))        PORT_NAME("\xe2\x86\x91")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 54 */ PORT_NAME("54")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 55 */ PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 56 */ PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 57 */ PORT_CODE(KEYCODE_COMMA_PAD)  PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 58 */ PORT_NAME("58")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 59 */ PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)            PORT_NAME("Left Shift")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5a */ PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5b */ PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5c */ PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5d */ PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5e */ PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5f */ PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("row_6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 60 */ PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 61 */ PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 62 */ PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 63 */ PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 64 */ PORT_CODE(KEYCODE_SLASH)     PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 65 */ PORT_CODE(KEYCODE_RSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)            PORT_NAME("Right Shift")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 66 */ PORT_NAME("66")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 67 */ PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN))      PORT_NAME("\xe2\x86\x93")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 68 */ PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     PORT_NAME("\xe2\x86\x92")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 69 */ PORT_CODE(KEYCODE_2_PAD)     PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6a */ PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6b */ PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6c */ PORT_NAME("6c")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6d */ PORT_NAME("6d")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6e */ PORT_NAME("6e Function?")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6f */ PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')

	PORT_START("row_7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 70 */ PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 71 */ PORT_CODE(KEYCODE_RCONTROL)                                  PORT_NAME("Line Feed  Home")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 72 */ PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 73 */ PORT_NAME("73")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 74 */ PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))  PORT_NAME("\xe2\x86\x90")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 75 */ PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 76 */ PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 77 */ PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 78 */ PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1))    PORT_NAME("PF1")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 79 */ PORT_NAME("79")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7a */ PORT_NAME("7a")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7b */ PORT_NAME("7b")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7c */ PORT_NAME("7c")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7d */ PORT_NAME("7d")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7e */ PORT_NAME("7e")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7f */ PORT_NAME("7f")
INPUT_PORTS_END

ioport_constructor f5220_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  f5220_kbd_device - constructor
//-------------------------------------------------

f5220_kbd_device::f5220_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, F5220_KBD, tag, owner, clock),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4", "row_5", "row_6", "row_7"),
	m_int_handler(*this),
	m_data(0x00)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void f5220_kbd_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();

	// timer for initial keyboard data
	m_reset_timer = timer_alloc(0);
	m_reset_timer->adjust(attotime::from_msec(1000));

	// register for state saving
	save_item(NAME(m_data));
}

//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------

void f5220_kbd_device::device_reset()
{
	reset_key_state();
	start_processing(attotime::from_hz(9600));
	typematic_stop();

	m_data = 0x00;
}

//-------------------------------------------------
//  device_timer - device-specific timer
//-------------------------------------------------

void f5220_kbd_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	logerror("Reset\n");

	m_int_handler(1);
	m_int_handler(0);
}

//-------------------------------------------------
//  read - external read from keyboard
//-------------------------------------------------

uint8_t f5220_kbd_device::read()
{
	return m_data;
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void f5220_kbd_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = translate(row, column);
	send_key(code);
	typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void f5220_kbd_device::key_break(uint8_t row, uint8_t column)
{
	if (typematic_is(row, column))
		typematic_stop();

	uint8_t code = translate(row, column);

	send_key(0x80 | code);
}

//-------------------------------------------------
//  key_repeat - handle a key being repeated
//-------------------------------------------------

void f5220_kbd_device::key_repeat(u8 row, u8 column)
{
	uint8_t code = translate(row, column);
	send_key(code);
}

//-------------------------------------------------
//  translate - row and column to key code
//-------------------------------------------------

uint8_t f5220_kbd_device::translate(uint8_t row, uint8_t column)
{
	return row * 16 + column;
}

//-------------------------------------------------
//  send_key - send key code to host
//-------------------------------------------------

void f5220_kbd_device::send_key(uint8_t code)
{
	m_data = code;
	m_int_handler(1);
	m_int_handler(0);
}
