// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    BASF 7100 Keyboard (HLE)

    Keytronics keyboard hardware: A65-01966-003 PCB-201 F
    BASF part number 19-4203-01

    Hardware:
    - 30293E-054 20-04592-054 (possibly PIC1650)
    - 1 KB ROM (contains translation tables)
    - Speaker

    TODO:
    - Extract internal ROM from MCU and convert to LLE
    - Speaker

***************************************************************************/

#include "emu.h"
#include "basf7100_kbd.h"
#include "machine/keyboard.ipp"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BASF7100_KBD, basf7100_kbd_device, "basf7100_kbd", "BASF 7100 Keyboard (HLE)")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( keyboard )
	ROM_REGION(0x400, "translation", 0)
	ROM_LOAD("19-2114-01e.bin", 0x000, 0x400, CRC(d694b5dd) SHA1(6262379ba565c1de072b2b21dc3141db1ec5129c))
ROM_END

const tiny_rom_entry *basf7100_kbd_device::device_rom_region() const
{
	return ROM_NAME(keyboard);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( keyboard )
	PORT_START("mod")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CTRL")  PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAPS")  PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE

	PORT_START("row_0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)     PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)   PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)         PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('{') PORT_CHAR('[')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)     PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)       PORT_CHAR('\t') // 0x89 with shift
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)     PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)     PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|') // actually ¦
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('}')  PORT_CHAR(']')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)         PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(UCHAR_MAMEKEY(ENTER)) PORT_NAME("RETURN")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD)) PORT_NAME("Keypad .")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)       PORT_CHAR(UCHAR_MAMEKEY(F10))   PORT_NAME("LF")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNUSED) // sends keycode for "-", but not on keyboard
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)     PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)     PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)   PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_NAME("DEL")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad Space")
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)      PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)      PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)      PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)    PORT_CHAR(UCHAR_MAMEKEY(F12))   PORT_NAME("BREAK")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)   PORT_CHAR(UCHAR_MAMEKEY(LALT))  PORT_NAME("FUNCT")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)  PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)        PORT_CHAR(UCHAR_MAMEKEY(F9))        PORT_NAME("CLEAR")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)    PORT_CHAR(UCHAR_MAMEKEY(INSERT))    PORT_NAME("INS CHAR")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)      PORT_CHAR(UCHAR_MAMEKEY(PGUP))      PORT_NAME("INS LINE")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))      PORT_NAME("\xe2\x86\x90") // ←
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_NAME("ENTER")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)     PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)   PORT_CHAR(UCHAR_MAMEKEY(END))  PORT_NAME("CLR EOL")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)    PORT_CHAR(UCHAR_MAMEKEY(UP))   PORT_NAME("\xe2\x86\x91") // ↑
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)  PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("\xe2\x86\x93") // ↓
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)  PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_NAME("HOME")
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_UNUSED) // sends keycode for TAB, but physical space taken by ENTER key
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("row_8")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)        PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)        PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)        PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)       PORT_CHAR(UCHAR_MAMEKEY(F8))        PORT_NAME("CLR EOS")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)      PORT_CHAR(UCHAR_MAMEKEY(DEL))       PORT_NAME("DEL CHAR")
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)     PORT_CHAR(UCHAR_MAMEKEY(PGDN))      PORT_NAME("DEL LINE")
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)    PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     PORT_NAME("\xe2\x86\x92") // →
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  PORT_NAME("SOM EOM")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor basf7100_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  basf7100_kbd_device - constructor
//-------------------------------------------------

basf7100_kbd_device::basf7100_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BASF7100_KBD, tag, owner, clock),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4", "row_5", "row_6", "row_7", "row_8"),
	m_translation(*this, "translation"),
	m_modifiers(*this, "mod"),
	m_int_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void basf7100_kbd_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();

	// register for state saving
	save_item(NAME(m_data));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void basf7100_kbd_device::device_reset()
{
	reset_key_state();
	start_processing(attotime::from_hz(1200));
	typematic_stop();
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void basf7100_kbd_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = translate(row, column);

	if (code != 0xff)
	{
		send_key(code);
		typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));
	}
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void basf7100_kbd_device::key_break(uint8_t row, uint8_t column)
{
	if (typematic_is(row, column))
		typematic_stop();
}

//-------------------------------------------------
//  key_repeat - handle a key being repeated
//-------------------------------------------------

void basf7100_kbd_device::key_repeat(u8 row, u8 column)
{
	uint8_t code = translate(row, column);
	send_key(code);
}

//-------------------------------------------------
//  translate - row and column to key code
//-------------------------------------------------

uint8_t basf7100_kbd_device::translate(uint8_t row, uint8_t column)
{
	uint8_t const modifiers(m_modifiers->read());

	bool const ctrl(modifiers & 0x01);
	bool const shift(bool(modifiers & 0x02) || (bool(modifiers & 0x04)));
	bool const ctrl_shift(ctrl && shift);

	unsigned const map(ctrl_shift? 3 : ctrl ? 2 : shift ? 1 : 0);

	return m_translation[(map * 0x100) + 0x10 + (row * 0x10) + column];
}

//-------------------------------------------------
//  send_key - send key code to host
//-------------------------------------------------

void basf7100_kbd_device::send_key(uint8_t code)
{
	m_data = code;
	m_int_handler(1);
	m_int_handler(0);
}

//-------------------------------------------------
//  read - return current keycode
//-------------------------------------------------

uint8_t basf7100_kbd_device::read()
{
	return m_data;
}
