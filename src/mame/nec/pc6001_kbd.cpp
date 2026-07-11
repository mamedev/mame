// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-6001 keyboard

Read indirectly thru the 8049 MCU.
On Mr. PC the keyboard is detached from the body, actually using an IR interface or a 4p4c phone
headset cable.

TODO:
- Mimicking some of the MCU actual internals here, if/when MCU LLE is dumped properly
  then this will just contain the input port def;
- CTRL, GRAPH, KANA, CAPS (no scancode?), INS and ROT (two rotating arrows on vanilla PC-6001);
- "<|--|> PAGE" (same as ROT?) and MODE on Mr. PC keyboards;

**************************************************************************************************/

#include "emu.h"
#include "machine/keyboard.ipp"
#include "pc6001_kbd.h"

#include "utf8.h"

DEFINE_DEVICE_TYPE(PC6001_KBD,   pc6001_kbd_device,   "pc6001_kbd",     "NEC PC-6001 Keyboard")

pc6001_kbd_device::pc6001_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC6001_KBD, tag, owner, clock)
	, device_matrix_keyboard_interface(mconfig, *this, "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8", "KEY9")
	, m_key_irq_cb(*this)
	, m_keyfn_irq_cb(*this)
	, m_joy_irq_cb(*this)
{
}

static INPUT_PORTS_START( pc6001_kbd )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
//	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
//	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("GRPH") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1 / F6") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CHAR('@')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2 / F7") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CHAR('_')

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3 / F8") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
 	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("^") PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4 / F9") PORT_CODE(KEYCODE_F4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("\xEF\xBF\xA5") PORT_CHAR(165) PORT_CHAR('|')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5 / F10") PORT_CODE(KEYCODE_F5)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(UCHAR_MAMEKEY(ENTER))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STOP")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("KEY9")
//	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KANA")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
//	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ROT")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HOME CLR") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

//	PORT_START("KEY_MOD")
//	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
INPUT_PORTS_END

ioport_constructor pc6001_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc6001_kbd );
}

void pc6001_kbd_device::device_start()
{
	m_joy_trigger_timer = timer_alloc(FUNC(pc6001_kbd_device::joy_trigger_cb), this);

	save_item(NAME(m_scan_code));
	save_item(NAME(m_joy_code));
}

void pc6001_kbd_device::device_reset()
{
	reset_key_state();
	start_processing(attotime::from_hz(1'200));
	typematic_stop();

	m_scan_code = 0x00;
}

uint8_t pc6001_kbd_device::translate(uint8_t row, uint8_t column)
{
	const u8 keytable[2][10 * 8] = {
		// normal
		{
//     [0]  ----   CTRL   SHIFT  GRAPH  ----   ----   ----   ----
			0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
//     [1]  1      Q      A      Z      K      I      8      ,
			0x31,  0x51,  0x41,  0x5a,  0x4b,  0x49,  0x38,  0x2c,
//     [2]  2      W      S      X      L      O      9      .
			0x32,  0x57,  0x53,  0x58,  0x4c,  0x4f,  0x39,  0x2e,
//     [3]  3      E      D      C      ;      P      F1     /
			0x33,  0x45,  0x44,  0x43,  0x3b,  0x50,  0xf0,  0x2f,
//     [4]  4      R      F      V      :      @      F2     _?
			0x34,  0x52,  0x46,  0x56,  0x3a,  0x40,  0xf1,  0x5f,
//     [5]  5      T      G      B      ]      [      F3     SPACE
			0x35,  0x54,  0x47,  0x42,  0x5d,  0x5b,  0xf2,  0x20,
//     [6]  6      Y      H      N      -      ^      F4     0
			0x36,  0x59,  0x48,  0x4e,  0x2d,  0x5e,  0xf3,  0x30,
//     [7]  7      U      J      M      -----  YEN?   F5     -----
			0x37,  0x55,  0x4a,  0x4d,  0x00,  0x5c,  0xf4,  0x00,
//     [8]  RET    STOP   UP     DOWN   RIGHT  LEFT   TAB    ESC?
			0x0d,  0xfa,  0x1e,  0x1f,  0x1c,  0x1d,  0x09,  0x1b,
//     [9]  KANA   INS?   DEL    ROT?   HOME   -----  -----  -----
			0x00,  0x00,  0x08,  0x00,  0x0b,  0x00,  0x00,  0x00
		},
		// shift
		{
//     [0]  ----   CTRL   SHIFT  GRAPH  ----   ----   ----   ----
			0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
//     [1]  !      q      a      z      k      i      8      <
			0x21,  0x71,  0x61,  0x7a,  0x6b,  0x69,  0x28,  0x3c,
//     [2]  "      w      s      x      l      o      )      >
			0x22,  0x77,  0x73,  0x78,  0x6c,  0x6f,  0x29,  0x3e,
//     [3]  #      e      d      c      +      p      F6     ?
			0x23,  0x65,  0x64,  0x63,  0x2b,  0x70,  0xf5,  0x3f,
//     [4]  $      r      f      v      *      @?     F7     _?
			0x24,  0x72,  0x66,  0x76,  0x2a,  0x40,  0xf6,  0x5f,
//     [5]  %      t      g      b      }      {      F8     SPACE
			0x25,  0x74,  0x67,  0x62,  0x7d,  0x7b,  0xf7,  0x20,
//     [6]  &      y      h      n      =      ~?     F9     0
			0x26,  0x79,  0x68,  0x6e,  0x3d,  0x7e,  0xf8,  0x30,
//     [7]  \      u      j      m      -----  |?     F10    -----
			0x27,  0x75,  0x6a,  0x6d,  0x00,  0x7c,  0xf9,  0x00,
//     [8]  RET    STOP   UP     DOWN   RIGHT  LEFT   TAB    ESC?
			0x0d,  0xfa,  0x1e,  0x1f,  0x1c,  0x1b,  0x09,  0x1b,
//     [9]  KANA   INS?   DEL    ROT?   CLR    ----   ----   ----
			0x00,  0x00,  0x08,  0x00,  0x0c,  0x00,  0x00,  0x00
		}
	};

	const u8 key_select = row * 8 + column;
	const uint8_t shift_key = BIT(m_key_rows[0]->read(), 2);

	return keytable[shift_key][key_select];
}

void pc6001_kbd_device::key_make(uint8_t row, uint8_t column)
{
	m_scan_code = translate(row, column);

	if (m_scan_code != 0x00)
	{
		if ((m_scan_code & 0xf0) == 0xf0)
			m_keyfn_irq_cb(1);
		else
			m_key_irq_cb(1);
	}
}

void pc6001_kbd_device::key_break(uint8_t row, uint8_t column)
{
	if ((m_scan_code & 0xf0) == 0xf0)
		m_keyfn_irq_cb(0);
	else
		m_key_irq_cb(0);

	m_scan_code = 0x00;
}

void pc6001_kbd_device::key_repeat(uint8_t row, uint8_t column)
{
	// ...
}

void pc6001_kbd_device::joy_cmd_w(int state)
{
	if (state)
	{
		// TODO: unknown timing, 0.1 msec is too short for pc6001:sharrier
		m_joy_trigger_timer->reset();
		m_joy_trigger_timer->adjust(attotime::from_usec(1666 * 2));
	}
}

// Rearrange keyboard key presses in a joystick like fashion (akin to Sharp X1)
// BIOS uses shift from this to determine function key display at bottom
uint8_t pc6001_kbd_device::convert_key_to_joy_map()
{
	const uint8_t space_key = BIT(m_key_rows[5]->read(), 7);
	const uint8_t shift_key = BIT(m_key_rows[0]->read(), 2);
	const uint8_t dir_keys = m_key_rows[8]->read() & 0x3c;
	uint8_t joy_press = 0;

	joy_press |= space_key << 7;
	joy_press |= dir_keys;
	joy_press |= shift_key;

	return joy_press;
}

TIMER_CALLBACK_MEMBER(pc6001_kbd_device::joy_trigger_cb)
{
	m_joy_code = convert_key_to_joy_map();
	m_joy_irq_cb(1);
	m_joy_irq_cb(0);
}
