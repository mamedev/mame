// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

SMC-777 keyboard interface

TODO:
- Really comes from a 8041A with undumped ROM, with a custom protocol (host doesn't see any
  serial-like i/f);
- Key repeat;
- Irq handling;
- Understand how multikey presses really works (currently worked around);
- Numpad is a separate slot interface (yagni without LLE dump I guess);

**************************************************************************************************/

#include "emu.h"
#include "smc777_kbd.h"
#include "machine/keyboard.ipp"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SMC777_KBD, smc777_kbd_device, "smc777_kbd", "Sony SMC-777 Keyboard HLE")

smc777_kbd_device::smc777_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SMC777_KBD, tag, owner, clock)
	, device_matrix_keyboard_interface(mconfig, *this, "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8", "KEY9")
	, m_key_mod(*this, "KEY_MOD")
	, m_caplock_cb(*this)
	, m_kanalock_cb(*this)
{
}

void smc777_kbd_device::device_start()
{
	m_aux_timer = timer_alloc(FUNC(smc777_kbd_device::aux_ready), this);
	m_aux_timer->adjust(attotime::never);
	m_aux_hs = 4;

	save_item(NAME(m_command));
	save_item(NAME(m_status));
	save_item(NAME(m_aux_hs));
	save_item(NAME(m_scan_code));
	save_item(NAME(m_repeat_interval));
	save_item(NAME(m_repeat_start));
	save_item(NAME(m_held_keys));
	save_item(NAME(m_fkey_table));
	save_item(NAME(m_fkey_index));
	save_item(NAME(m_fkey_target));
	save_item(NAME(m_fkey_dir));
}


void smc777_kbd_device::device_reset()
{
	reset_key_state();
	start_processing(attotime::from_hz(1'200));
	typematic_stop();
	// don't reset aux timer: we do that separatedly

	m_command = 0;
	m_status = 0;
	m_scan_code = 0;
	m_repeat_interval = 100;
	m_repeat_start = 1000;
	m_held_keys = 0;

	// reset fkeys to default
	m_fkey_dir = false;
	m_fkey_index = 0;
	m_fkey_target = 0xf;

	const u8 fkey_default_table[3][6] =
	{
		/* normal*/
		{
			0x01, 0x02, 0x04, 0x06, 0x0b, 0x1d
		},
		/* shift */
		{
			0x15, 0x18, 0x12, 0x05, 0x03, 0x1d
		},
		/* ctrl */
		{
			0x1a, 0x10, 0x13, 0x07, 0x0c, 0x1d
		}
	};
	for (int j = 0; j < 3; j++)
		for (int i = 0; i < 6; i++)
			m_fkey_table[j][i] = fkey_default_table[j][i];
}

INPUT_CHANGED_MEMBER(smc777_kbd_device::cap_changed)
{
	m_caplock_cb(newval);
}

INPUT_CHANGED_MEMBER(smc777_kbd_device::kana_changed)
{
	m_kanalock_cb(newval);
}

static INPUT_PORTS_START( smc777_kbd )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // TEN_DN
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(". / >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) // TEN_UP
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LF") PORT_CODE(KEYCODE_F12)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("/ / ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(", / <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CURSOR DOWN") PORT_CODE(KEYCODE_DOWN)
	// Red 'H', on the right of Kana Lock
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_F11)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"¥ / ¦") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(U'¥','\\') PORT_CHAR(U'¦','|')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CURSOR UP") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CURSOR LEFT") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; / :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CURSOR RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("` / ~")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\' / \"")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("] / }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("[ / {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RUBOUT")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("= / +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) // TEN_LT
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("9 / (") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("7 / &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("5 / %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("3 / #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("1 / !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // TEN_RT
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("- / _") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("0 / )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("8 / *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("6 / ^") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("4 / $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("2 / @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CLR") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)

	PORT_START("KEY_MOD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	// printed without the 'S'
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CAP LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(smc777_kbd_device::cap_changed), 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KANA LOCK") PORT_CODE(KEYCODE_LALT) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(smc777_kbd_device::kana_changed), 0)
INPUT_PORTS_END

ioport_constructor smc777_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( smc777_kbd );
}

//**************************************************************************
//  device_matrix_keyboard
//**************************************************************************

uint8_t smc777_kbd_device::translate(uint8_t row, uint8_t column)
{
	// Ignore numpad for now
	// F-keys handled dynamically therefore ignored in this static table
	const u8 keytable[5][0x50] =
	{
		/* normal */
		{
//          ------, TEN_DN, RETURN, >,      TEN_UP, SPACE,  E,      ESC
			0xff,   0xff,   0x0d,   0x2e,   0xff,   0x20,   0x65,   0x1b,
//          LF,     ------, ------, ?,      <,      N,      V,      X
			0x0a,   0xff,   0xff,   0x2f,   0x2c,   0x6e,   0x76,   0x78,
//          CUR_DN, HELP,   \,      ------, M,      B,      C,      Z
			0x1c,   0xff,   0x5c,   0xff,   0x6d,   0x62,   0x63,   0x7a,
//          CUR_UP, CUR_LT, ------, :,      K,      H,      F,      S
			0x17,   0x16,   0xff,   0x3b,   0x6b,   0x68,   0x66,   0x73,
//          CUR_RT, ~,      ",      L,      J,      G,      D,      A
			0x19,   0x60,   0x27,   0x6c,   0x6a,   0x67,   0x64,   0x61,
//          BS,     },      {,      O,      U,      T,      ------, Q
			0x08,   0x5d,   0x5b,   0x6f,   0x75,   0x74,   0xff,   0x71,
//          ------, RUBOUT, P,      I,      Y,      R,      W,      TAB
			0xff,   0x7f,   0x70,   0x69,   0x79,   0x72,   0x77,   0x09,
//          HOME,   +,      TEN_LT, 9,      7,      5,      3,      1
			0x14,   0x3d,   0xff,   0x39,   0x37,   0x35,   0x33,   0x31,
//          DEL,    TEN_RT, -,      0,      8,      6,      4,      2
			0x11,   0xff,   0x2d,   0x30,   0x38,   0x36,   0x34,   0x32,
//          INS,    CLR,    ------, F1,     F2,     F3,     F4,     F5
			0x0f,   0x0e,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff
		},
		/* shift */
		{
//          ------, TEN_DN, RETURN, >,      TEN_UP, SPACE,  E,      ESC
			0xff,   0xff,   0x0d,   0x3e,   0xff,   0x20,   0x45,   0x1b,
//          LF,     ------, ------, ?,      <,      N,      V,      X
			0x0a,   0xff,   0xff,   0x3f,   0x3c,   0x4e,   0x56,   0x58,
//          CUR_DN, HELP,   \,      ------, M,      B,      C,      Z
			0x1c,   0xff,   0x7c,   0xff,   0x4d,   0x42,   0x43,   0x5a,
//          CUR_UP, CUR_LT, ------, :,      K,      H,      F,      S
			0x17,   0x16,   0xff,   0x3a,   0x4b,   0x48,   0x46,   0x53,
//          CUR_RT, ~,      ",      L,      J,      G,      D,      A
			0x19,   0x7e,   0x22,   0x4c,   0x4a,   0x47,   0x44,   0x41,
//          BS,     },      {,      O,      U,      T,      ------, Q
			0x08,   0x7d,   0x7b,   0x4f,   0x55,   0x54,   0xff,   0x51,
//          ------, RUBOUT, P,      I,      Y,      R,      W,      TAB
			0xff,   0x7f,   0x50,   0x49,   0x59,   0x52,   0x57,   0x09,
//          HOME,   +,      TEN_LT, 9,      7,      5,      3,      1
			0x14,   0x2b,   0xff,   0x28,   0x26,   0x25,   0x23,   0x21,
//          DEL,    TEN_RT, -,      0,      8,      6,      4,      2
			0x11,   0xff,   0x5f,   0x29,   0x2a,   0x5e,   0x24,   0x40,
//          INS,    CLR,    ------, F1,     F2,     F3,     F4,     F5
			0x0f,   0x0e,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff
		},
		/* kana */
		{
//          ------, TEN_DN, RETURN, >,      TEN_UP, SPACE,  E,      ESC
			0xff,   0xff,   0x0d,   0xa6,   0xff,   0x20,   0xb8,   0x1b,
//          LF,     ------, ------, ?,      <,      N,      V,      X
			0x0a,   0xff,   0xff,   0xff,   0xd6,   0xd4,   0xc3,   0xc1,
//          CUR_DN, HELP,   \,      ------, M,      B,      C,      Z
			0x1c,   0xff,   0xdd,   0xff,   0xd5,   0xc4,   0xc2,   0xc0,
//          CUR_UP, CUR_LT, ------, :,      K,      H,      F,      S
			0x17,   0x16,   0xff,   0xd1,   0xd1,   0xcf,   0xbe,   0xbc,
//          CUR_RT, ~,      ",      L,      J,      G,      D,      A
			0x19,   0xdf,   0xde,   0xd2,   0xd0,   0xbf,   0xbd,   0xbb,
//          BS,     },      {,      O,      U,      T,      ------, Q
			0x08,   0xdb,   0xda,   0xcd,   0xcb,   0xba,   0xff,   0xb6,
//          ------, RUBOUT, P,      I,      Y,      R,      W,      TAB
			0xff,   0xd9,   0xce,   0xcc,   0xca,   0xb9,   0xb7,   0x09,
//          HOME,   +,      TEN_LT, 9,      7,      5,      3,      1
			0x14,   0xd8,   0xff,   0xc8,   0xc6,   0xb5,   0xb3,   0xb1,
//          DEL,    TEN_RT, -,      0,      8,      6,      4,      2
			0x11,   0xff,   0xd7,   0xc9,   0xc7,   0xc5,   0xb4,   0xb2,
//          INS,    CLR,    ------, F1,     F2,     F3,     F4,     F5
			0x0f,   0x0e,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff
		},
		/* kana shift */
		{
//          ------, TEN_DN, RETURN, >,      TEN_UP, SPACE,  E,      ESC
			0xff,   0xff,   0x0d,   0xa1,   0xff,   0x20,   0xb8,   0x1b,
//          LF,     ------, ------, ?,      <,      N,      V,      X
			0x0a,   0xff,   0xff,   0xff,   0xae,   0xac,   0xc3,   0xc1,
//          CUR_DN, HELP,   \,      ------, M,      B,      C,      Z
			0x1c,   0xff,   0xa5,   0xff,   0xad,   0xc4,   0xaf,   0xc0,
//          CUR_UP, CUR_LT, ------, :,      K,      H,      F,      S
			0x17,   0x16,   0xff,   0xd1,   0xd1,   0xcf,   0xbe,   0xbc,
//          CUR_RT, ~,      ",      L,      J,      G,      D,      A
			0x19,   0xa3,   0xa2,   0xd2,   0xd0,   0xbf,   0xbd,   0xbb,
//          BS,     },      {,      O,      U,      T,      ------, Q
			0x08,   0xb0,   0xda,   0xcd,   0xcb,   0xba,   0xff,   0xb6,
//          ------, RUBOUT, P,      I,      Y,      R,      W,      TAB
			0xff,   0xd9,   0xce,   0xcc,   0xca,   0xb9,   0xb7,   0x09,
//          HOME,   +,      TEN_LT, 9,      7,      5,      3,      1
			0x14,   0xd8,   0xff,   0xc8,   0xc6,   0xab,   0xa9,   0xa7,
//          DEL,    TEN_RT, -,      0,      8,      6,      4,      2
			0x11,   0xff,   0xd7,   0xc9,   0xc7,   0xc5,   0xaa,   0xa8,
//          INS,    CLR,    ------, F1,     F2,     F3,     F4,     F5
			0x0f,   0x0e,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff
		},
		/* ctrl */
		{
//          ------, TEN_DN, RETURN, >,      TEN_UP, SPACE,  E,      ESC
			0xff,   0xff,   0x0d,   0x1e,   0xff,   0x20,   0x05,   0x1b,
//          LF,     ------, ------, ?,      <,      N,      V,      X
			0x0a,   0xff,   0xff,   0x1f,   0x00,   0x0e,   0x16,   0x18,
//          CUR_DN, HELP,   \,      ------, M,      B,      C,      Z
			0x1c,   0xff,   0x1c,   0xff,   0x0d,   0x02,   0x03,   0x1a,
//          CUR_UP, CUR_LT, ------, :,      K,      H,      F,      S
			0x17,   0x16,   0xff,   0x00,   0x0b,   0x08,   0x06,   0x13,
//          CUR_RT, ~,      ",      L,      J,      G,      D,      A
			0x19,   0x00,   0x00,   0x0c,   0x0a,   0x07,   0x04,   0x01,
//          BS,     },      {,      O,      U,      T,      ------, Q
			0x08,   0x1d,   0x1b,   0x0f,   0x15,   0x14,   0xff,   0x11,
//          ------, RUBOUT, P,      I,      Y,      R,      W,      TAB
			0xff,   0x7f,   0x10,   0x09,   0x19,   0x12,   0x17,   0x09,
//          HOME,   +,      TEN_LT, 9,      7,      5,      3,      1
			0x14,   0x00,   0xff,   0x00,   0x00,   0x00,   0x00,   0x00,
//          DEL,    TEN_RT, -,      0,      8,      6,      4,      2
			0x11,   0xff,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
//          INS,    CLR,    ------, F1,     F2,     F3,     F4,     F5
			0x0f,   0x0e,   0xff,   0xff,   0xff,   0xff,   0xff,   0xff
		}
	};

	const u8 caps_modifier[0x50] = {
//      ------, TEN_DN, RETURN, >,      TEN_UP, SPACE,  E,      ESC
		0,      0,      0,      0,      0,      0,      1,      0,
//      LF,     ------, ------, ?,      <,      N,      V,      X
		0,      0,      0,      0,      0,      1,      1,      1,
//      CUR_DN, HELP,   \,      ------, M,      B,      C,      Z
		0,      0,      0,      0,      1,      1,      1,      1,
//      CUR_UP, CUR_LT, ------, :,      K,      H,      F,      S
		0,      0,      0,      0,      1,      1,      1,      1,
//      CUR_RT, ~,      ",      L,      J,      G,      D,      A
		0,      0,      0,      1,      1,      1,      1,      1,
//      BS,     },      {,      O,      U,      T,      ------, Q
		0,      0,      0,      1,      1,      1,      0,      1,
//      ------, RUBOUT, P,      I,      Y,      R,      W,      TAB
		0,      0,      1,      1,      1,      1,      1,      0,
//      HOME,   +,      TEN_LT, 9,      7,      5,      3,      1
		0,      0,      0,      0,      0,      0,      0,      0,
//      DEL,    TEN_RT, -,      0,      8,      6,      4,      2
		0,      0,      0,      0,      0,      0,      0,      0,
//      INS,    CLR,    ------, F1,     F2,     F3,     F4,     F5
		0,      0,      0,      0,      0,      0,      0,      0
	};

	const u8 key_mod = m_key_mod->read();
	const u8 shift = BIT(key_mod, 0);
	const u8 ctrl = BIT(key_mod, 1);
	u8 modifier = ctrl ? 4 : shift ? 1 : 0;

	// HELP key
	if (row == 2 && column == 1)
		return m_fkey_table[modifier][5];

	// F1 ~ F5 keys
	if (row == 9 && column >= 3)
		return m_fkey_table[modifier][column - 3];

	const u8 key_select = row * 8 + column;

	// check for locks if CTRL not pressed
	if ((modifier & 0xfc) == 0)
	{
		// KANA LOCK
		if (BIT(key_mod, 3))
		{
			modifier |= 2;
		}
		// CAP(S) LOCK, applies to A-Z keys only
		else if (BIT(key_mod, 2) && caps_modifier[key_select] == 1)
		{
			modifier ^= 1;
		}
	}

	return keytable[modifier][key_select];
}


void smc777_kbd_device::key_make(uint8_t row, uint8_t column)
{
	m_scan_code = translate(row, column);
	m_status |= 5;
	//printf("%d %d\n", row, column);
	// take count of multipresses like this (testable in games like ldrun/cloderun)
	m_held_keys++;
}

void smc777_kbd_device::key_break(uint8_t row, uint8_t column)
{
	m_held_keys--;
	if (m_held_keys <= 0)
	{
		// clear code for safety
		// (would cause drift in stuff like hudson1~5, where pressing enter at MAME startup
		//  will get stuck from POST up to game selection)
		m_scan_code = 0;
		m_status &= ~4;
	}
}

void smc777_kbd_device::key_repeat(uint8_t row, uint8_t column)
{
	// ...
}

//**************************************************************************
//  Interface
//**************************************************************************

void smc777_kbd_device::scan_mode(u8 data)
{
	// ICF: clear irq
	if (BIT(data, 7))
	{
		m_status &= ~1;
		LOG("Irq ack\n");
	}
	// FEF: enter setting mode
	if (BIT(data, 4))
	{
		LOG("Enter setting mode\n");
		m_command = 0xff;
	}
	// IEF: enable interrupt
	if (BIT(data, 0))
	{
		LOG("Enable ief_key (TBD)\n");
	}
}

u8 smc777_kbd_device::data_r(offs_t offset)
{
	if (m_command == 0)
	{
		// TODO: really cleared after 80 usec
		if (!machine().side_effects_disabled())
			m_status &= ~1;
		return m_scan_code;
	}
	else if (m_command == 0x80 && !m_fkey_dir)
	{
		LOG("data_r function key code %01x\n", m_fkey_target);
		if (m_fkey_target == 0xf)
			return 0;
		u8 res = m_fkey_table[m_fkey_index][m_fkey_target];
		if (!machine().side_effects_disabled())
		{
			m_fkey_index++;
			if (m_fkey_index >= 3)
				m_command = 0xff;
		}
		return res;
	}

	return 0;
}

void smc777_kbd_device::data_w(offs_t offset, u8 data)
{
	if (m_command == 0)
		scan_mode(data);
	else if (m_command == 0x80 && m_fkey_dir)
	{
		LOG("data_w function key code %01x %02x\n", m_fkey_target, data);
		if (m_fkey_target == 0xf)
			return;
		m_fkey_table[m_fkey_index][m_fkey_target] = data;
		m_fkey_index++;
		if (m_fkey_index >= 3)
			m_command = 0xff;
	}
	else
		LOG("data_w with command %02x data %02x (ignored)\n", m_command, data);
}

/*
 * In scan mode:
 * x--- ---- CF  CTRL pressed
 * -x-- ---- SF  SHIFT pressed
 * ---- -x-- ASF key pressed now
 * ---- ---x BSF key was pressed (released on data read)
 * In setting mode:
 * ---- -x-- /BUSY command done if 1
 * ---- --x- /CS command accepted if 0
 * ---- ---x DR data ready if 1
 */
u8 smc777_kbd_device::status_r(offs_t offset)
{
	u8 res = 0;
	if (m_command == 0)
	{
		res = m_status;
		res|= (m_key_mod->read() & 3) << 6;
	}
	else
	{
		res = m_aux_hs | (m_command == 0x80);
	}

	return res;
}

void smc777_kbd_device::control_w(offs_t offset, u8 data)
{
	if (m_command == 0)
	{
		scan_mode(data);
	}
	else
	{
		m_command = data & 0xc0;
		LOG("Command %02x - %02x\n", m_command, data & 0x3f);
		switch(m_command)
		{
			case 0x00:
				LOG("Exit setting mode\n");
				break;

			case 0x40:
				if (BIT(data, 5))
				{
					m_repeat_start = 500 + (100 * (data & 0xf));
					LOG("Key repeat start %d\n", m_repeat_start);
				}
				else
				{
					m_repeat_interval = 20 + (20 * (data & 0xf));
					LOG("Key repeat interval %d\n", m_repeat_interval);
				}
				break;

			case 0x80:
				m_fkey_target = data & 0xf;
				m_fkey_dir = !!BIT(data, 5);
				m_fkey_index = 0;
				LOG("Function key %s target %01x\n", m_fkey_dir ? "write" : "read", m_fkey_target);
				// clamp to natural table, otherwise mark it as invalid
				// (is the MCU really handling f-keys starting from bit 1?)
				if (m_fkey_target == std::clamp<u8>(m_fkey_target, 1, 6))
					m_fkey_target--;
				else
					m_fkey_target = 0xf;
				break;

			case 0xc0:
				this->reset();
				LOG("Initialize key\n");
				break;
		}
		m_aux_hs &= ~4;
		// TODO: timing
		m_aux_timer->adjust(attotime::from_usec(250));
	}
}

// - bugatk expects BUSY to go low otherwise space key won't work during gameplay
TIMER_CALLBACK_MEMBER(smc777_kbd_device::aux_ready)
{
	m_aux_hs |= 4;
}
