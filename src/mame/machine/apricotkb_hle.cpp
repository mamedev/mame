// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Keyboard (HLE)

    Keyboard to System:
    - 01-60: Key make codes
    - 70-7f: Mouse codes
    - 80: RAM test failed
    - 81-e0: Key break codes
    - e9: ROM test failed
    - ea: X-on (keyboard ready to receive)
    - eb: X-off (keyboard buffer full)
    - ec: Reset request
    - ed: Time prefix
    - ee: Date prefix
    - ef: Mouse header
    - f0-f9: BCD data
    - fa: Invalid clock data
    - fb: Acknowledge firmware version/reset

    System to keyboard:
    - 01-7f: Character codes for MicroScreen
    - 80-cf: Cursor address
    - d0: Clear screen
    - d1: Cursor left
    - d2: Cursor right
    - d3: Cursor on
    - d4: Cursor off
    - d5: Display on
    - d6: Display off
    - e0: Query
    - e1: Time and date request
    - e2: Display time/data on MicroScreen
    - e3: Set LED prefix
    - e4: Set time and date
    - e5: Mouse enable
    - e6: Mouse disable
    - e7: Execute processor diagnostics
    - e8: Keyboard reset
    - f0-f9: BCD data
    - fa: Invalid clock data

***************************************************************************/

#include "apricotkb_hle.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define APRICOT_KEY(_key, _index) \
		PORT_BIT(1 << _key, IP_ACTIVE_HIGH, IPT_KEYBOARD) \
		PORT_CHANGED_MEMBER(DEVICE_SELF, apricot_keyboard_hle_device, key_callback, (void *) _index)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type APRICOT_KEYBOARD_HLE = &device_creator<apricot_keyboard_hle_device>;


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( keyboard )
	PORT_START("keyboard_0")
	PORT_BIT(1 << 0, IP_ACTIVE_HIGH, IPT_UNUSED)
	APRICOT_KEY( 1, 0) PORT_CODE(KEYCODE_F7)        PORT_CHAR(UCHAR_MAMEKEY(F7))     PORT_NAME("Help")
	APRICOT_KEY( 2, 0) PORT_CODE(KEYCODE_F8)        PORT_CHAR(UCHAR_MAMEKEY(F8))     PORT_NAME("Undo")
	APRICOT_KEY( 3, 0) PORT_CODE(KEYCODE_F9)        PORT_CHAR(UCHAR_MAMEKEY(F9))     PORT_NAME("Repeat")
	APRICOT_KEY( 4, 0) PORT_CODE(KEYCODE_F10)       PORT_CHAR(UCHAR_MAMEKEY(F10))    PORT_NAME("Calc")
	APRICOT_KEY( 5, 0) PORT_CODE(KEYCODE_PRTSCR)    PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) PORT_NAME("Print")
	APRICOT_KEY( 6, 0) PORT_CODE(KEYCODE_CANCEL)    PORT_CHAR(UCHAR_MAMEKEY(CANCEL)) PORT_NAME("Intr")
	APRICOT_KEY( 7, 0) PORT_CODE(KEYCODE_F11)       PORT_CHAR(UCHAR_MAMEKEY(F11))    PORT_NAME("Menu")
	APRICOT_KEY( 8, 0) PORT_CODE(KEYCODE_F12)       PORT_CHAR(UCHAR_MAMEKEY(F12))    PORT_NAME("Finish")
	APRICOT_KEY( 9, 0) PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1))     PORT_NAME("Function 1")
	APRICOT_KEY(10, 0) PORT_CODE(KEYCODE_F2)        PORT_CHAR(UCHAR_MAMEKEY(F2))     PORT_NAME("Function 2")
	APRICOT_KEY(11, 0) PORT_CODE(KEYCODE_F3)        PORT_CHAR(UCHAR_MAMEKEY(F3))     PORT_NAME("Function 3")
	APRICOT_KEY(12, 0) PORT_CODE(KEYCODE_F4)        PORT_CHAR(UCHAR_MAMEKEY(F4))     PORT_NAME("Function 4")
	APRICOT_KEY(13, 0) PORT_CODE(KEYCODE_F5)        PORT_CHAR(UCHAR_MAMEKEY(F5))     PORT_NAME("Function 5")
	APRICOT_KEY(14, 0) PORT_CODE(KEYCODE_F6)        PORT_CHAR(UCHAR_MAMEKEY(F6))     PORT_NAME("Function 6")
	APRICOT_KEY(15, 0) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('^')
	APRICOT_KEY(16, 0) PORT_CODE(KEYCODE_1)         PORT_CHAR('1')  PORT_CHAR('!')
	APRICOT_KEY(17, 0) PORT_CODE(KEYCODE_2)         PORT_CHAR('2')  PORT_CHAR('@')
	APRICOT_KEY(18, 0) PORT_CODE(KEYCODE_3)         PORT_CHAR('3')  PORT_CHAR('#')
	APRICOT_KEY(19, 0) PORT_CODE(KEYCODE_4)         PORT_CHAR('4')  PORT_CHAR(0xa3) // pound
	APRICOT_KEY(20, 0) PORT_CODE(KEYCODE_5)         PORT_CHAR('5')  PORT_CHAR('%')
	APRICOT_KEY(21, 0) PORT_CODE(KEYCODE_6)         PORT_CHAR('6')  PORT_CHAR('$')
	APRICOT_KEY(22, 0) PORT_CODE(KEYCODE_7)         PORT_CHAR('7')  PORT_CHAR('&')
	APRICOT_KEY(23, 0) PORT_CODE(KEYCODE_8)         PORT_CHAR('8')  PORT_CHAR('*')
	APRICOT_KEY(24, 0) PORT_CODE(KEYCODE_9)         PORT_CHAR('9')  PORT_CHAR('(')
	APRICOT_KEY(25, 0) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')  PORT_CHAR(')')
	APRICOT_KEY(26, 0) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-')  PORT_CHAR('_')
	APRICOT_KEY(27, 0) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('=')  PORT_CHAR('+')
	APRICOT_KEY(28, 0) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(9)
	PORT_BIT(1 << 29, IP_ACTIVE_HIGH, IPT_UNUSED) // actually a dedicated % key
	APRICOT_KEY(30, 0) PORT_CODE(KEYCODE_ASTERISK)  PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	APRICOT_KEY(31, 0) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))

	PORT_START("keyboard_1")
	APRICOT_KEY( 0, 1) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	APRICOT_KEY( 1, 1) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	APRICOT_KEY( 2, 1) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	APRICOT_KEY( 3, 1) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	APRICOT_KEY( 4, 1) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	APRICOT_KEY( 5, 1) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	APRICOT_KEY( 6, 1) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	APRICOT_KEY( 7, 1) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	APRICOT_KEY( 8, 1) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	APRICOT_KEY( 9, 1) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	APRICOT_KEY(10, 1) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	APRICOT_KEY(11, 1) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	APRICOT_KEY(12, 1) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	APRICOT_KEY(13, 1) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	APRICOT_KEY(14, 1) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	APRICOT_KEY(15, 1) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	APRICOT_KEY(16, 1) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))      PORT_NAME("Clear")
	APRICOT_KEY(17, 1) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	APRICOT_KEY(18, 1) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	APRICOT_KEY(19, 1) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	APRICOT_KEY(20, 1) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	APRICOT_KEY(21, 1) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')   PORT_CHAR('A')
	APRICOT_KEY(22, 1) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')   PORT_CHAR('S')
	APRICOT_KEY(23, 1) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')   PORT_CHAR('D')
	APRICOT_KEY(24, 1) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')   PORT_CHAR('F')
	APRICOT_KEY(25, 1) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')   PORT_CHAR('G')
	APRICOT_KEY(26, 1) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')   PORT_CHAR('H')
	APRICOT_KEY(27, 1) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')   PORT_CHAR('J')
	APRICOT_KEY(28, 1) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')   PORT_CHAR('K')
	APRICOT_KEY(29, 1) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')   PORT_CHAR('L')
	APRICOT_KEY(30, 1) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')   PORT_CHAR(':')
	APRICOT_KEY(31, 1) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'')  PORT_CHAR('"')

	PORT_START("keyboard_2")
	APRICOT_KEY( 0, 2) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	APRICOT_KEY( 1, 2) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	APRICOT_KEY( 2, 2) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	APRICOT_KEY( 3, 2) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	APRICOT_KEY( 4, 2) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	APRICOT_KEY( 5, 2) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	APRICOT_KEY( 6, 2) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	APRICOT_KEY( 7, 2) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	APRICOT_KEY( 8, 2) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	APRICOT_KEY( 9, 2) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	APRICOT_KEY(10, 2) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	APRICOT_KEY(11, 2) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	APRICOT_KEY(12, 2) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')
	APRICOT_KEY(13, 2) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	APRICOT_KEY(14, 2) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	APRICOT_KEY(15, 2) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	APRICOT_KEY(16, 2) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	APRICOT_KEY(17, 2) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	APRICOT_KEY(18, 2) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	APRICOT_KEY(19, 2) PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))
	APRICOT_KEY(20, 2) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	APRICOT_KEY(21, 2) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	APRICOT_KEY(22, 2) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	APRICOT_KEY(23, 2) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	APRICOT_KEY(24, 2) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)
	APRICOT_KEY(25, 2) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	APRICOT_KEY(26, 2) PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(PAUSE)) PORT_NAME("Stop")
	APRICOT_KEY(27, 2) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	APRICOT_KEY(28, 2) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	APRICOT_KEY(29, 2) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	APRICOT_KEY(30, 2) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	APRICOT_KEY(31, 2) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("keyboard_3")
	APRICOT_KEY( 0, 3) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
INPUT_PORTS_END

ioport_constructor apricot_keyboard_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  apricot_keyboard_hle_device - constructor
//-------------------------------------------------

apricot_keyboard_hle_device::apricot_keyboard_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, APRICOT_KEYBOARD_HLE, "Apricot Keyboard (HLE)", tag, owner, clock, "apricotkb_hle", __FILE__),
	device_serial_interface(mconfig, *this),
	m_txd_handler(*this),
	m_rxd(1),
	m_data_in(0),
	m_data_out(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_keyboard_hle_device::device_start()
{
	// resolve callbacks
	m_txd_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apricot_keyboard_hle_device::device_reset()
{
	receive_register_reset();
	transmit_register_reset();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(7800);
	set_tra_rate(7800);
}

//-------------------------------------------------
//  device_timer - device-specific timer
//-------------------------------------------------

void apricot_keyboard_hle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}

void apricot_keyboard_hle_device::tra_callback()
{
	m_txd_handler(transmit_register_get_data_bit());
}

void apricot_keyboard_hle_device::tra_complete()
{
	if (m_data_out != 0)
	{
		transmit_register_setup(m_data_out);
		m_data_out = 0;
	}
}

void apricot_keyboard_hle_device::rcv_callback()
{
	receive_register_update_bit(m_rxd);
}

void apricot_keyboard_hle_device::rcv_complete()
{
	receive_register_extract();
	m_data_in = get_received_char();

	// reset command? send keyboard ready (likely needs a delay, just disable for now)
//  if (m_data_in == 0xe8)
//      transmit_register_setup(0xfb);
}

WRITE_LINE_MEMBER( apricot_keyboard_hle_device::rxd_w )
{
	m_rxd = state;
	device_serial_interface::rx_w(m_rxd);
}

INPUT_CHANGED_MEMBER( apricot_keyboard_hle_device::key_callback )
{
	UINT32 oldvalue = oldval * field.mask(), newvalue = newval * field.mask();
	UINT32 delta = oldvalue ^ newvalue;

	for (int i = 0; i < 32; i++)
	{
		if (delta & (1 << i))
		{
			UINT8 down = (newvalue & (1 << i)) ? 0x00 : 0x80;
			UINT8 scancode = (FPTR) param * 32 + i;
			scancode |= down;
			transmit_register_setup(scancode);
			break;
		}
	}

}
