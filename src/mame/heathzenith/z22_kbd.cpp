// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

    Zenith Z-22 keyboard (high level emulation)

    The keyboard ROM hasn't been dumped, so this emulates the protocol the
    terminal firmware expects, worked out from the terminal ROMs:

    - 300 baud, 8 data bits, no parity, 1 stop bit, in both directions
    - bytes 00h-7Fh from the terminal are echoed back: it sends 55h at power
      up and shows KEYBOARD ERROR without a reply, and keyboard test #2
      sends every value
    - DFh is followed by a one byte command (05h/06h/07h seen; probably
      click and repeat settings, not decoded)
    - each key sends one code, which the terminal translates through a
      256 entry table in its ROM (CA00h, or CC00h for the alternate keypad)
    - codes below 80h are the main keys, with shift already applied using
      a bit-paired layout (32h = '2', 22h = '@')
    - 80h-BFh are the keypad, C0h-E9h the editing and cursor keys (bit 5 set
      when shifted)
    - CTRL sends C5h when pressed (E5h with shift) and D5h when released;
      the terminal applies control itself
    - holding CTRL and pressing SET UP (D1h) twice enters setup, or resets
      the terminal when already in setup; CTRL and SET UP once leaves the
      diagnostic screens

    The key layout follows photos of a Z-22 keyboard. The codes of the
    keys without an ASCII meaning are inferred from how the firmware treats
    them, and could be wrong:
    - TAB C6h/E6h: the only codes that give 09h, and shifted is back tab
    - HOME/CLEAR C7h/E3h: C7h gives ^U (home, with the LOC/TX HOME setup
      options) and C3h/E3h ^W (erase, with the LOC/TX ERASE options)
    - cursor keys: 88h is a keypad-style code distinct from BACK SPACE,
      so it's taken as left; up C2h (^K), right C9h (^L), down D8h (^V,
      the TeleVideo code), leaving C4h (^J) for LINE FEED
    - F1-F10 F0h-F9h: the alternate keypad table maps these to A-J, as for
      programmable function keys
    - keypad - and , send plain 2Dh and 2Ch

    The self-test (KEYBOARD TEST #1) names C8h/C6h/C4h/C2h/C0h F1-F5 and
    C1h/C3h/C7h/C9h INT/ATT/KBD/DSP, which doesn't match this keyboard,
    so it was probably written for an earlier one.

    TODO:
    - codes for HELP and FA-FD (declared, but send nothing)
    - keyboard commands (click, repeat)
    - confirm the guessed codes against a real keyboard or its ROM

***************************************************************************/

#include "emu.h"
#include "z22_kbd.h"

#include "machine/keyboard.ipp"

#define LOG_CMD (1U << 1)
#define LOG_KEY (1U << 2)

//#define VERBOSE (LOG_CMD | LOG_KEY)
#include "logmacro.h"

#define LOGCMD(...) LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGKEY(...) LOGMASKED(LOG_KEY, __VA_ARGS__)


DEFINE_DEVICE_TYPE(Z22_KEYBOARD, z22_keyboard_device, "z22_kbd", "Zenith Z-22 Keyboard (HLE)")

namespace {

// special entries in the key table
constexpr u8 KEY_NONE   = 0x00;
constexpr u8 KEY_CTRL   = 0x01;
constexpr u8 KEY_LSHIFT = 0x02;
constexpr u8 KEY_RSHIFT = 0x03;
constexpr u8 KEY_CAPS   = 0x04;

// unshifted and shifted code for each matrix position; { 0, 0 } sends nothing
constexpr u8 KEY_CODES[7][16][2] =
{
	{   // row 0
		{ 0x31, 0x21 }, { 0x32, 0x22 }, { 0x33, 0x23 }, { 0x34, 0x24 },  // 1! 2@ 3# 4$
		{ 0x35, 0x25 }, { 0x36, 0x5d }, { 0x37, 0x27 }, { 0x38, 0x28 },  // 5% 6^ 7& 8*
		{ 0x39, 0x29 }, { 0x30, 0x00 }, { 0x2d, 0x3d }, { 0x5f, 0x7f },  // 9( 0) -_ =+
		{ 0x7e, 0x7d }, { 0x08, 0x08 }, { 0x1b, 0x1b }, { 0xc6, 0xe6 },  // `~ BS ESC TAB
	},
	{   // row 1
		{ 0x71, 0x51 }, { 0x77, 0x57 }, { 0x65, 0x45 }, { 0x72, 0x52 },  // q w e r
		{ 0x74, 0x54 }, { 0x79, 0x59 }, { 0x75, 0x55 }, { 0x69, 0x49 },  // t y u i
		{ 0x6f, 0x4f }, { 0x70, 0x50 }, { 0x40, 0x7c }, { 0x60, 0x5c },  // o p [{ ]}
		{ 0x5e, 0x26 }, { 0x7b, 0x7b }, { 0x0d, 0x0d },                  // \| DEL RETURN
	},
	{   // row 2
		{ 0x61, 0x41 }, { 0x73, 0x53 }, { 0x64, 0x44 }, { 0x66, 0x46 },  // a s d f
		{ 0x67, 0x47 }, { 0x68, 0x48 }, { 0x6a, 0x4a }, { 0x6b, 0x4b },  // g h j k
		{ 0x6c, 0x4c }, { 0x3b, 0x2b }, { 0x3a, 0x2a },                  // l ;: '"
	},
	{   // row 3
		{ 0x7a, 0x5a }, { 0x78, 0x58 }, { 0x63, 0x43 }, { 0x76, 0x56 },  // z x c v
		{ 0x62, 0x42 }, { 0x6e, 0x4e }, { 0x6d, 0x4d }, { 0x2c, 0x3c },  // b n m ,<
		{ 0x2e, 0x3e }, { 0x2f, 0x3f }, { 0x20, 0x20 },                  // .> /? SPACE
	},
	{   // row 4: keypad
		{ 0xb0, 0xb0 }, { 0xb1, 0xb1 }, { 0xb2, 0xb2 }, { 0xb3, 0xb3 },
		{ 0xb4, 0xb4 }, { 0xb5, 0xb5 }, { 0xb6, 0xb6 }, { 0xb7, 0xb7 },
		{ 0xb8, 0xb8 }, { 0xb9, 0xb9 }, { 0xae, 0xae }, { 0x8d, 0x8d },  // . ENTER
		{ 0x2d, 0x2d }, { 0x2c, 0x2c },                                  // - ,
	},
	{   // row 5: cursor and editing keys, modifiers
		{ 0xc2, 0xe2 }, { 0xd8, 0xd8 }, { 0x88, 0x88 }, { 0xc9, 0xe9 },  // up down left right
		{ KEY_CTRL, KEY_CTRL }, { KEY_LSHIFT, KEY_LSHIFT }, { KEY_RSHIFT, KEY_RSHIFT }, { KEY_CAPS, KEY_CAPS },
		{ 0xd1, 0xd1 }, { 0xc4, 0xe4 }, { 0xc7, 0xe3 },                  // SET UP, LINE FEED, HOME/CLEAR
	},
	{   // row 6: function keys
		{ 0xf0, 0xf0 }, { 0xf1, 0xf1 }, { 0xf2, 0xf2 }, { 0xf3, 0xf3 },  // F1-F4
		{ 0xf4, 0xf4 }, { 0xf5, 0xf5 }, { 0xf6, 0xf6 }, { 0xf7, 0xf7 },  // F5-F8
		{ 0xf8, 0xf8 }, { 0xf9, 0xf9 },                                  // F9 F10
		{ 0x00, 0x00 },                                                  // HELP (code unknown)
		{ 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 },  // FA-FD (codes unknown)
	},
};

constexpr bool is_modifier(u8 code)
{
	return (code >= KEY_CTRL) && (code <= KEY_CAPS);
}

// SET UP doesn't repeat (holding CTRL and pressing it twice enters setup)
constexpr bool no_repeat(u8 code)
{
	return code == 0xd1;
}

} // anonymous namespace


z22_keyboard_device::z22_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, Z22_KEYBOARD, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6")
	, m_txd_cb(*this)
{
}

static INPUT_PORTS_START(z22_keyboard)
	PORT_START("ROW0")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)

	PORT_START("ROW1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_NAME("Keypad ,")
	PORT_BIT(0xc000, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)        PORT_NAME("Set Up")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)       PORT_NAME("Line Feed") PORT_CHAR(10)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_NAME("Home  Clear") PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0xf800, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_NAME("Help")
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_NAME("FA")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_NAME("FB")
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_NAME("FC")
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_NAME("FD")
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ioport_constructor z22_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(z22_keyboard);
}

void z22_keyboard_device::device_start()
{
	save_item(NAME(m_shift_left));
	save_item(NAME(m_shift_right));
	save_item(NAME(m_caps));
	save_item(NAME(m_command));
}

void z22_keyboard_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(300);
	set_tra_rate(300);
	receive_register_reset();
	transmit_register_reset();
	clear_fifo();

	m_txd_cb(1);

	m_shift_left = false;
	m_shift_right = false;
	m_caps = false;
	m_command = false;

	reset_key_state();
	start_processing(attotime::from_hz(600));
	typematic_stop();
}

void z22_keyboard_device::tra_callback()
{
	m_txd_cb(transmit_register_get_data_bit());
}

void z22_keyboard_device::received_byte(u8 byte)
{
	if (m_command)
	{
		LOGCMD("command %02x\n", byte);
		m_command = false;
	}
	else if (byte < 0x80)
	{
		// echoed: the terminal sends 55h at power up, and 00h-7Fh in keyboard test #2
		transmit_byte(byte);
	}
	else if (byte == 0xdf)
	{
		m_command = true;
	}
	else
	{
		LOGCMD("unknown byte %02x\n", byte);
	}
}

u8 z22_keyboard_device::key_code(u8 row, u8 column) const
{
	bool const shifted = m_shift_left || m_shift_right;
	u8 code = KEY_CODES[row][column][shifted ? 1 : 0];

	// caps lock only affects letters
	if (m_caps && (code >= 0x61) && (code <= 0x7a))
		code &= ~0x20;

	return code;
}

void z22_keyboard_device::key_make(u8 row, u8 column)
{
	u8 const code = KEY_CODES[row][column][0];

	switch (code)
	{
	case KEY_CTRL:
		transmit_byte((m_shift_left || m_shift_right) ? 0xe5 : 0xc5);
		return;
	case KEY_LSHIFT:
		m_shift_left = true;
		return;
	case KEY_RSHIFT:
		m_shift_right = true;
		return;
	case KEY_CAPS:
		m_caps = !m_caps;
		return;
	}

	// ')' sends 00h, so only the unshifted code marks an empty position
	if (code == KEY_NONE)
		return;

	LOGKEY("make %u,%u -> %02x\n", row, column, key_code(row, column));
	transmit_byte(key_code(row, column));
	if (!no_repeat(code))
		typematic_start(row, column, attotime::from_msec(500), attotime::from_msec(66));
}

void z22_keyboard_device::key_repeat(u8 row, u8 column)
{
	LOGKEY("repeat %u,%u -> %02x\n", row, column, key_code(row, column));
	transmit_byte(key_code(row, column));
}

void z22_keyboard_device::key_break(u8 row, u8 column)
{
	u8 const code = KEY_CODES[row][column][0];

	if (is_modifier(code))
	{
		switch (code)
		{
		case KEY_CTRL:
			transmit_byte(0xd5);
			break;
		case KEY_LSHIFT:
			m_shift_left = false;
			break;
		case KEY_RSHIFT:
			m_shift_right = false;
			break;
		}
		return;
	}

	if (typematic_is(row, column))
		typematic_stop();
}
