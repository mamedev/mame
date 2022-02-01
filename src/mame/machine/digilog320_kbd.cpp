// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Digilog 320 Keyboard (HLE)

    TODO:
    - Map and verify missing keys

***************************************************************************/

#include "emu.h"
#include "digilog320_kbd.h"
#include "machine/keyboard.ipp"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DIGILOG320_KBD_HLE, digilog320_kbd_hle_device, "digilog320_kbd_hle", "Digilog 320 Keyboard (HLE)")

namespace {

uint8_t const TRANSLATION_TABLE[4][5][18] =
{
	// unshift
	{
		{ 0x00,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', 0x00, 0x8f, 0x96, 0x95, 0x80 }, // 0
		{ 0x8a,  'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', 0x8d, 0x8e, 0x89, 0x97, 0x88 }, // 1
		{ 0x00,  'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', 0x00, 0x00, 0x84, 0x85, 0x81, 0x90, 0x00 }, // 2
		{ 0x00, '\\',  'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '/', 0x00, 0x98, 0x99, 0x91, 0x92, 0x93 }, // 3
		{ 0x00, 0x00,  ' ', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  '`', '\'', 0x87, 0x8b, 0x8c, 0x83, 0x94, 0x82 }, // 4
	},
	// shift
	{
		{ 0x00,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+', 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x00,  'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00,  'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2
		{ 0x00,  '|',  'Z',  'X',  'C',  'V',  'B',  'N',  'M',  '<',  '>',  '?', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  '~',  '"', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
	},
	// unshift-control
	{
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
	},
	// shift-control
	{
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 3
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 4
	}
};

} // anonymous namespace

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

// unknown codes: Nul, Return, Halt, Mask / Crc, Don't Care (0x83?), Not (0x82?)

static INPUT_PORTS_START( keyboard )
	PORT_START("mod")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl")      PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift")     PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK)                             PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE

	PORT_START("row_0")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)     PORT_NAME("Nul")
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)      PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)      PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)      PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)      PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)      PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)      PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)      PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)      PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)      PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)  PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)     PORT_NAME("Halt")
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)     PORT_NAME("Run Mon")
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)     PORT_NAME("Run Trap")
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)     PORT_NAME("Run Prog")
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)     PORT_NAME("Auto Set-up")

	PORT_START("row_1")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
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
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_NAME("Page Fwd")
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_NAME("Page Rev")
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Main Menu")
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_NAME("Exec Menu Cmd")
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_NAME("Hex")

	PORT_START("row_2")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_UNUSED) // "Ctrl" handled elsewhere
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
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)        PORT_NAME("Field Sel Fwd")
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)       PORT_NAME("Field Sel Rev")
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)       PORT_NAME("Mask  Crc")

	PORT_START("row_3")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_UNUSED) // "Left Shift" handled elsewhere
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_UNUSED) // "Right Shift" handled elsewhere
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_NAME("Tab Fwd")
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)        PORT_NAME("Tab Rev")
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_NAME("Freeze  Home")
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("row_4")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_UNUSED) // "Shift Lock"
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_UNUSED) // "Caps Lock" handled elsewhere
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)  PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)  PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)    PORT_NAME("Help")
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("Insert Line Del")
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)  PORT_NAME("Insert Char Del")
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)  PORT_NAME("Don\'t Care")
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)   PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)  PORT_NAME("Not")
INPUT_PORTS_END

ioport_constructor digilog320_kbd_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  digilog320_kbd_hle_device - constructor
//-------------------------------------------------

digilog320_kbd_hle_device::digilog320_kbd_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, DIGILOG320_KBD_HLE, tag, owner, clock),
	device_buffered_serial_interface(mconfig, *this),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4"),
	m_modifiers(*this, "mod"),
	m_tx_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void digilog320_kbd_hle_device::device_start()
{
	// resolve callbacks
	m_tx_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void digilog320_kbd_hle_device::device_reset()
{
	clear_fifo();

	receive_register_reset();
	transmit_register_reset();

	set_data_frame(1, 8, PARITY_EVEN, STOP_BITS_1);
	set_rcv_rate(1200);
	set_tra_rate(1200);

	reset_key_state();
	start_processing(attotime::from_hz(1200));
	typematic_stop();
}

//-------------------------------------------------
//  tra_callback - send bit to host
//-------------------------------------------------

void digilog320_kbd_hle_device::tra_callback()
{
	m_tx_handler(transmit_register_get_data_bit());
}

//-------------------------------------------------
//  received_byte - handle received byte
//-------------------------------------------------

void digilog320_kbd_hle_device::received_byte(uint8_t byte)
{
	// might be send only
	logerror("Received from host: %02x\n", byte);
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void digilog320_kbd_hle_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t code = translate(row, column);

	if (code != 0x00)
	{
		transmit_byte(code);
		typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));
	}
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void digilog320_kbd_hle_device::key_break(uint8_t row, uint8_t column)
{
	if (typematic_is(row, column))
		typematic_stop();
}

//-------------------------------------------------
//  key_repeat - handle a key being repeated
//-------------------------------------------------

void digilog320_kbd_hle_device::key_repeat(u8 row, u8 column)
{
	uint8_t code = translate(row, column);

	if (code != 0x00)
		transmit_byte(code);
}

//-------------------------------------------------
//  rx_w - receive bit from host
//-------------------------------------------------

void digilog320_kbd_hle_device::rx_w(int state)
{
	device_buffered_serial_interface::rx_w(state);
}

//-------------------------------------------------
//  translate - row and column to key code
//-------------------------------------------------

uint8_t digilog320_kbd_hle_device::translate(uint8_t row, uint8_t column)
{
	uint8_t const modifiers(m_modifiers->read());

	bool const ctrl(modifiers & 0x01);
	bool const shift(bool(modifiers & 0x02) || (bool(modifiers & 0x04)));
	bool const ctrl_shift(ctrl && shift);

	unsigned const map(ctrl_shift? 3 : ctrl ? 2 : shift ? 1 : 0);

	return TRANSLATION_TABLE[map][row][column];
}
