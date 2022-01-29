// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Informer 213 Keyboard (HLE)

***************************************************************************/

#include "emu.h"
#include "informer_213_kbd.h"
#include "machine/keyboard.ipp"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(INFORMER_213_KBD_HLE, informer_213_kbd_hle_device, "in213kbd_hle", "Informer 213 Keyboard (HLE)")


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( keyboard )
	PORT_START("row_0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 00 */ PORT_NAME("0x00")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 01 */ PORT_NAME("0x01")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 02 */ PORT_NAME("0x02")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 03 */ PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 04 */ PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 05 */ PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 06 */ PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 07 */ PORT_CODE(KEYCODE_END)        PORT_NAME("ERASE EOF  ERASE INPUT")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 08 */ PORT_CODE(KEYCODE_F5)         PORT_NAME("PF5  PF17")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 09 */ PORT_NAME("0x09")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0a */ PORT_NAME("0x0a")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0b */ PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0c */ PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0d */ PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0e */ PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 0f */ PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')

	PORT_START("row_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 10 */ PORT_CODE(KEYCODE_F6)         PORT_NAME("PF6  PF18")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 11 */ PORT_NAME("0x11")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 12 */ PORT_NAME("0x12")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 13 */ PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 14 */ PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 15 */ PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 16 */ PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 17 */ PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('{') PORT_CHAR('}')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 18 */ PORT_CODE(KEYCODE_F7)         PORT_NAME("PF7  PF19")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 19 */ PORT_NAME("0x19")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1a */ PORT_CODE(KEYCODE_ENTER_PAD)  PORT_NAME("ENTER  SET UP")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1b */ PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1c */ PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1d */ PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1e */ PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 1f */ PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')

	PORT_START("row_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 20 */ PORT_CODE(KEYCODE_F8)         PORT_NAME("PF8  PF20")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 21 */ PORT_NAME("0x21")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 22 */ PORT_CODE(KEYCODE_ENTER)      PORT_NAME("\xe2\x86\xb2") // ↲ (LF)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 23 */ PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 24 */ PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 25 */ PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 26 */ PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 27 */ PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|') // (actually ¦)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 28 */ PORT_CODE(KEYCODE_F9)         PORT_NAME("PF9  PF21")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 29 */ PORT_NAME("0x29")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2a */ PORT_CODE(KEYCODE_RIGHT)      PORT_NAME("\xe2\x86\x92") // →
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2b */ PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2c */ PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2d */ PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2e */ PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 2f */ PORT_CODE(KEYCODE_INSERT)     PORT_NAME("â  DUP  PA1")

	PORT_START("row_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 30 */ PORT_CODE(KEYCODE_F10)        PORT_NAME("PF10  PF22")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 31 */ PORT_NAME("0x31")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 32 */ PORT_CODE(KEYCODE_DOWN)       PORT_NAME("\xe2\x86\x93") // ↓
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 33 */ PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 34 */ PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 35 */ PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 36 */ PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 37 */ PORT_CODE(KEYCODE_DEL)        PORT_NAME("DEL  FIELD MARK  PA2")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 38 */ PORT_CODE(KEYCODE_F11)        PORT_NAME("PF11  PF23")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 39 */ PORT_NAME("0x39")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3a */ PORT_CODE(KEYCODE_HOME)       PORT_NAME("KEYCLICK  TEST")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3b */ PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3c */ PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3d */ PORT_CODE(KEYCODE_BACKSPACE)  PORT_NAME("\xe2\x87\xa4  HOME") // ⇤
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3e */ PORT_CODE(KEYCODE_LEFT)       PORT_NAME("\xe2\x86\x90") // ←
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 3f */ PORT_CODE(KEYCODE_UP)         PORT_NAME("\xe2\x86\x91") // ↑

	PORT_START("row_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 40 */ PORT_CODE(KEYCODE_F12)        PORT_NAME("PF12  PF24")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 41 */ PORT_CODE(KEYCODE_TAB)        PORT_NAME("\xe2\x87\xa5") // ⇥
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 42 */ PORT_NAME("0x42")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 43 */ PORT_CODE(KEYCODE_ESC)        PORT_NAME("RESET  DEV CNCL")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 44 */ PORT_NAME("0x44")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 45 */ PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 46 */ PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 47 */ PORT_CODE(KEYCODE_PAUSE)      PORT_NAME("ATTN  PA3  SYS REQ")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 48 */ PORT_CODE(KEYCODE_F1)         PORT_NAME("PF1  PF13")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 49 */ PORT_NAME("0x49")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4a */ PORT_NAME("0x4a")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4b */ PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4c */ PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4d */ PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4e */ PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 4f */ PORT_CODE(KEYCODE_PGUP)       PORT_NAME("CURSOR SEL  CLEAR")

	PORT_START("row_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 50 */ PORT_CODE(KEYCODE_F2)         PORT_NAME("PF2  PF14")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 51 */ PORT_NAME("0x51")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 52 */ PORT_NAME("0x52")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 53 */ PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 54 */ PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 55 */ PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 56 */ PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 57 */ PORT_CODE(KEYCODE_PGDN)       PORT_NAME("CURSOR BLINK  ALT CURSOR")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 58 */ PORT_CODE(KEYCODE_F3)         PORT_NAME("PF3  PF15")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 59 */ PORT_NAME("0x59")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5a */ PORT_NAME("0x5a")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5b */ PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5c */ PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5d */ PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5e */ PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 5f */ PORT_CODE(KEYCODE_BACKSLASH2) PORT_NAME("PRINT  IDENT") // but in setup Pound/Cent?

	PORT_START("row_6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 60 */ PORT_CODE(KEYCODE_F4)         PORT_NAME("PF4  PF16")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 61 */ PORT_NAME("0x61")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 62 */ PORT_NAME("0x62")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 63 */ PORT_NAME("0x63")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 64 */ PORT_NAME("0x64")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 65 */ PORT_NAME("0x65")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 66 */ PORT_NAME("0x66")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 67 */ PORT_NAME("0x67")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 68 */ PORT_NAME("0x68")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 69 */ PORT_NAME("0x69")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6a */ PORT_NAME("0x6a")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6b */ PORT_NAME("0x6b")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6c */ PORT_NAME("0x6c")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6d */ PORT_NAME("0x6d")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6e */ PORT_NAME("0x6e")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 6f */ PORT_NAME("0x6f")

	PORT_START("row_7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 70 */ PORT_NAME("0x70")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 71 */ PORT_NAME("0x71")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 72 */ PORT_NAME("0x72")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 73 */ PORT_NAME("0x73")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 74 */ PORT_NAME("0x74")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 75 */ PORT_NAME("0x75")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 76 */ PORT_NAME("0x76")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 77 */ PORT_NAME("0x77")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 78 */ PORT_NAME("0x78")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 79 */ PORT_NAME("0x79")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7a */ PORT_NAME("0x7a")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7b */ PORT_NAME("0x7b")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7c */ PORT_NAME("0x7c")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7d */ PORT_NAME("0x7d")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7e */ PORT_NAME("0x7e")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 7f */ PORT_NAME("0x7f")

	PORT_START("row_8")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM)   /* 80 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 81 */ PORT_CODE(KEYCODE_LALT)     PORT_NAME("ALT")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 82 */ PORT_CODE(KEYCODE_LSHIFT)   PORT_NAME("SHIFT (LEFT?)")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM)   /* 83 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 84 */ PORT_CODE(KEYCODE_CAPSLOCK) PORT_NAME("LOCK")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_CUSTOM)   /* 85 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM)   /* 86 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM)   /* 87 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) /* 88 */ PORT_CODE(KEYCODE_RSHIFT)   PORT_NAME("SHIFT (RIGHT?)")
INPUT_PORTS_END

ioport_constructor informer_213_kbd_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  informer_213_kbd_hle_device - constructor
//-------------------------------------------------

informer_213_kbd_hle_device::informer_213_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, INFORMER_213_KBD_HLE, tag, owner, clock),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4", "row_5", "row_6", "row_7", "row_8"),
	m_int_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void informer_213_kbd_hle_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void informer_213_kbd_hle_device::device_reset()
{
	m_key = 0xff;
	m_mod = 0;

	reset_key_state();
	start_processing(attotime::from_hz(2400));
	typematic_stop();
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void informer_213_kbd_hle_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = row * 16 + column;

	// no typematic for modifier keys
	if (code < 0x80)
		typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));

	// modifier key (but not caps lock)
	if (code > 0x80 && code != 0x84)
	{
		m_mod |= code;
		code = m_mod;
	}

	// send the code
	m_key = code;
	m_int_handler(1);
	m_int_handler(0);
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void informer_213_kbd_hle_device::key_break(uint8_t row, uint8_t column)
{
	uint8_t code = row * 16 + column;

	if (typematic_is(row, column))
		typematic_stop();

	// send the break code (only for modifier keys: SHIFT and ALT)
	if (code > 0x80 && code != 0x84)
	{
		m_mod = 0x80;
		m_key = m_mod;
		m_int_handler(1);
		m_int_handler(0);
	}
}

//-------------------------------------------------
//  key_repeat - handle a key being repeated
//-------------------------------------------------

void informer_213_kbd_hle_device::key_repeat(u8 row, u8 column)
{
	uint8_t code = row * 16 + column;
	m_key = code;
	m_int_handler(1);
	m_int_handler(0);
}

//-------------------------------------------------
//  read - read data from keyboard
//-------------------------------------------------

uint8_t informer_213_kbd_hle_device::read()
{
	uint8_t tmp = m_key;
	m_key = 0xff;

	return tmp;
}

//-------------------------------------------------
//  write - write data to keyboard
//-------------------------------------------------

void informer_213_kbd_hle_device::write(uint8_t data)
{
	logerror("Keyboard received: %02x\n", data);
}
