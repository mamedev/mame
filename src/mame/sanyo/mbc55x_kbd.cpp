// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Serial keyboard emulation for Sanyo MBC-55x.

    The key matrix is based on schematics, though the MCU is not yet dumped.
    The Graph, Lock, Num Lock and 00 keys are not emulated yet, nor are the
    Lock and Graph key LEDs and the alternate keypad functions. The unknown
    jumper might select an international key layout.

    The interface has only one unidirectional communication line. The reset
    switch appears to be part of the main unit rather than the keyboard. The
    ASCII codes sent by this keyboard are converted into XT codes by MS-DOS
    using the lookup tables at the beginning of the ROM.

*******************************************************************************/

#include "emu.h"
#include "mbc55x_kbd.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/keyboard.ipp"

//**************************************************************************
//  DEVICE TYPE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(MBC55X_KEYBOARD, mbc55x_keyboard_device, "mbc55x_kbd", "MBC-55x Keyboard")

mbc55x_keyboard_device::mbc55x_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MBC55X_KEYBOARD, tag, owner, clock)
	, device_matrix_keyboard_interface(mconfig, *this, "Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y10", "Y11")
	, device_serial_interface(mconfig, *this)
	, m_modifiers(*this, "BUS")
	, m_txd_callback(*this)
{
}

//**************************************************************************
//  INPUT MATRIX
//**************************************************************************

static INPUT_PORTS_START(mbc55x_kbd)
	PORT_START("Y0")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK)) PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('<') PORT_CHAR('>') PORT_CODE(KEYCODE_BACKSLASH2)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF1  PF6") PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CODE(KEYCODE_F1)

	PORT_START("Y1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) /*PORT_NAME("\xe2\x87\x90")*/ PORT_CHAR(0x08) PORT_CODE(KEYCODE_BACKSPACE) // ⇐
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) /*PORT_NAME("\xe2\x86\xb9")*/ PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB) // ↹
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF2  PF7") PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CODE(KEYCODE_F2)

	PORT_START("Y2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Del  Ins") PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) /*PORT_NAME("\xe2\x8f\x8e")*/ PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER) // ⏎
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF3  PF8") PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CODE(KEYCODE_F3)

	PORT_START("Y3")
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF4  PF9") PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F9)) PORT_CODE(KEYCODE_F4)

	PORT_START("Y4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 4  " UTF8_LEFT) PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PF5  PF10") PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHAR(UCHAR_MAMEKEY(F10)) PORT_CODE(KEYCODE_F5)

	PORT_START("Y5")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 5  " UTF8_DOWN) PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(00_PAD)) PORT_CODE(KEYCODE_00_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD)) PORT_CODE(KEYCODE_EQUALS_PAD)

	PORT_START("Y6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 6  " UTF8_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD)) PORT_CODE(KEYCODE_SLASH_PAD)

	PORT_START("Y7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 7  \xe2\x86\x96") PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD) // ↖
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 1  LF") PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK)) PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("Y8")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 8  " UTF8_UP) PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",  ,") PORT_CHAR(',') PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_F15)

	PORT_START("Y9")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 9  Pg Up") PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Keypad 3  Pg Dn") PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".  .") PORT_CHAR('.') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_F15)

	PORT_START("Y10")
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)

	PORT_START("Y11")
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE) // to right of quote
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CHAR('*') PORT_CODE(KEYCODE_RCONTROL) // between slash and right shift
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) /*PORT_NAME("Keypad \xe2\x8f\x8e")*/ PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_ENTER_PAD) // ⏎

	PORT_START("BUS")
	PORT_BIT(0x8f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_DIPNAME(0x40, 0x00, DEF_STR(Unknown)) PORT_DIPLOCATION("JP1:1")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END

ioport_constructor mbc55x_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mbc55x_kbd);
}

//**************************************************************************
//  INITIALIZATION AND HLE SERIAL INTERFACE
//**************************************************************************

void mbc55x_keyboard_device::device_resolve_objects()
{
	m_txd_callback.resolve_safe();
}

void mbc55x_keyboard_device::device_start()
{
	set_tra_rate(1230); // more or less
}

void mbc55x_keyboard_device::device_reset()
{
	start_processing(attotime::from_hz(6_MHz_XTAL / 75)); // guess
	m_txd_callback(1);
}

void mbc55x_keyboard_device::tra_callback()
{
	m_txd_callback(transmit_register_get_data_bit());
}

void mbc55x_keyboard_device::tra_complete()
{
	start_processing(attotime::from_hz(6_MHz_XTAL / 75));
}

//**************************************************************************
//  KEYBOARD SCAN AND DECODING
//**************************************************************************

const u8 mbc55x_keyboard_device::s_code_table[2][12][8] =
{
	{
		{ 0x15, '>',  'A',  'Q',  '!',  0x00, 0x1b, '|'  },
		{ 0x16, 'Z',  'S',  'W',  '@',  ' ',  0x05, 0x08 },
		{ 0x17, 'X',  'D',  'E',  '#',  0x00, 0x0d, 0x07 },
		{ 0x18, 'C',  'F',  'R',  '$',  0x00, 0x00, 0x00 },
		{ 0x19, 'V',  'G',  'T',  '%',  '0',  '4',  0x00 },
		{ '=',  'B',  'H',  'Y',  '^',  '0',  '5',  0x00 },
		{ '/',  'N',  'J',  'U',  '&',  '.',  '6',  0x00 },
		{ '*',  'M',  'K',  'I',  '*',  '1',  '7',  0x00 },
		{ 0x03, ',',  'L',  'O',  '(',  '2',  '8',  0x00 },
		{ '-',  '.',  ':',  'P',  ')',  '3',  '9',  0x00 },
		{ '+',  '?',  '"',  '{',  '_',  0x00, 0x00, 0x00 },
		{ 0x0d, '*',  '~',  '}',  '+',  0x00, 0x00, 0x00 }
	},
	{
		{ 0x10, '<',  'a',  'q',  '1',  0x00, 0x1b, '\\' },
		{ 0x11, 'z',  's',  'w',  '2',  ' ',  0x09, 0x08 },
		{ 0x12, 'x',  'd',  'e',  '3',  0x00, 0x0d, 0x7f },
		{ 0x13, 'c',  'f',  'r',  '4',  0x00, 0x00, 0x00 },
		{ 0x14, 'v',  'g',  't',  '5',  '0',  '4',  0x00 },
		{ '=',  'b',  'h',  'y',  '6',  '0',  '5',  0x00 },
		{ '/',  'n',  'j',  'u',  '7',  '.',  '6',  0x00 },
		{ '*',  'm',  'k',  'i',  '8',  '1',  '7',  0x00 },
		{ 0x03, ',',  'l',  'o',  '9',  '2',  '8',  0x00 },
		{ '-',  '.',  ';',  'p',  '0',  '3',  '9',  0x00 },
		{ '+',  '/',  '\'', '[',  '-',  0x00, 0x00, 0x00 },
		{ 0x0d, '*',  '`',  ']',  '=',  0x00, 0x00, 0x00 }
	}
};

void mbc55x_keyboard_device::key_make(u8 row, u8 column)
{
	send_translated(row, column);
}

void mbc55x_keyboard_device::key_repeat(u8 row, u8 column)
{
	send_translated(row, column);
}

void mbc55x_keyboard_device::scan_complete()
{
	if (!is_transmit_register_empty())
		stop_processing();
}

void mbc55x_keyboard_device::send_key(u8 code, bool ctrl_active)
{
	if (!is_transmit_register_empty())
		return;

	// Control characters are transmitted with inverted parity
	set_data_frame(1, 8, ctrl_active ? PARITY_ODD : PARITY_EVEN, STOP_BITS_2);

	transmit_register_setup(code);
}

void mbc55x_keyboard_device::send_translated(u8 row, u8 column)
{
	const ioport_value modifiers = m_modifiers->read();

	u8 code = s_code_table[BIT(modifiers, 4)][row][column];
	if (code != 0)
		send_key(code, !BIT(modifiers, 5));
}

//**************************************************************************
//  KEYBOARD CONTROLLER CONFIGURATION
//**************************************************************************

void mbc55x_keyboard_device::device_add_mconfig(machine_config &config)
{
	I8049(config, "kbdc", 6_MHz_XTAL).set_disable();
	//kbdc.bus_in_cb().set_ioport("BUS");
	//kbdc.p1_in_cb().set(FUNC(mbc55x_keyboard_device::key_matrix_r));
	//kbdc.p2_out_cb().set(FUNC(mbc55x_keyboard_device::scan_output_w));
}

ROM_START(mbc55x_kbd)
	ROM_REGION(0x0800, "kbdc", 0)
	ROM_LOAD("d8049hc.m1", 0x0000, 0x0800, NO_DUMP)
ROM_END

const tiny_rom_entry *mbc55x_keyboard_device::device_rom_region() const
{
	return ROM_NAME(mbc55x_kbd);
}
