// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Keyboard (HLE)

    TODO:
    - MicroScreen emulation
    - LEDs

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

#include "emu.h"
#include "hle.h"
#include "machine/keyboard.ipp"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(APRICOT_KEYBOARD_HLE, apricot_keyboard_hle_device, "apricotkb_hle", "Apricot Keyboard (HLE)")


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( keyboard )
	PORT_START("row_0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))      PORT_NAME("Help")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))      PORT_NAME("Undo")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))      PORT_NAME("Repeat")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))     PORT_NAME("Calc")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))  PORT_NAME("Print")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_CANCEL)     PORT_CHAR(UCHAR_MAMEKEY(CANCEL))  PORT_NAME("Intr")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))     PORT_NAME("Menu")

	PORT_START("row_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))     PORT_NAME("Finish")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))      PORT_NAME("Function 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))      PORT_NAME("Function 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))      PORT_NAME("Function 3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))      PORT_NAME("Function 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))      PORT_NAME("Function 5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))      PORT_NAME("Function 6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('^')

	PORT_START("row_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR(0xa3) // pound
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('$')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*')

	PORT_START("row_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  // actually a dedicated % key
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))

	PORT_START("row_4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')

	PORT_START("row_5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))

	PORT_START("row_6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))      PORT_NAME("Clear")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_A)          PORT_CHAR('a')   PORT_CHAR('A')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_S)          PORT_CHAR('s')   PORT_CHAR('S')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_D)          PORT_CHAR('d')   PORT_CHAR('D')

	PORT_START("row_7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F)          PORT_CHAR('f')   PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_G)          PORT_CHAR('g')   PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_H)          PORT_CHAR('h')   PORT_CHAR('H')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_J)          PORT_CHAR('j')   PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_K)          PORT_CHAR('k')   PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_L)          PORT_CHAR('l')   PORT_CHAR('L')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')   PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'')  PORT_CHAR('"')

	PORT_START("row_8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')

	PORT_START("row_9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')

	PORT_START("row_a")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("row_b")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(PAUSE))    PORT_NAME("Stop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_START("row_c")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))

	PORT_START("mouse_b")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Mouse Right Button")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Left Button")
	PORT_BIT(0x0c, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("mouse_x")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(75) PORT_KEYDELTA(5)

	PORT_START("mouse_y")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)
INPUT_PORTS_END

ioport_constructor apricot_keyboard_hle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}

void apricot_keyboard_hle_device::device_add_mconfig(machine_config &config)
{
	MSM5832(config, m_rtc, 32.768_kHz_XTAL);

	TIMER(config, "timer").configure_periodic(FUNC(apricot_keyboard_hle_device::mouse_callback), attotime::from_hz(60));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  apricot_keyboard_hle_device - constructor
//-------------------------------------------------

apricot_keyboard_hle_device::apricot_keyboard_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APRICOT_KEYBOARD_HLE, tag, owner, clock),
	device_apricot_keyboard_interface(mconfig, *this),
	device_buffered_serial_interface(mconfig, *this),
	device_matrix_keyboard_interface(mconfig, *this, "row_0", "row_1", "row_2", "row_3", "row_4", "row_5", "row_6", "row_7", "row_8", "row_9", "row_a", "row_b", "row_c"),
	m_rtc(*this, "rtc"),
	m_mouse_b(*this, "mouse_b"),
	m_mouse_x(*this, "mouse_x"),
	m_mouse_y(*this, "mouse_y"),
	m_rtc_index(0),
	m_mouse_enabled(false),
	m_mouse_last_b(0), m_mouse_last_x(0), m_mouse_last_y(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_keyboard_hle_device::device_start()
{
	// register for save states
	save_item(NAME(m_rtc_index));
	save_item(NAME(m_mouse_enabled));
	save_item(NAME(m_mouse_last_b));
	save_item(NAME(m_mouse_last_x));
	save_item(NAME(m_mouse_last_y));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apricot_keyboard_hle_device::device_reset()
{
	clear_fifo();

	receive_register_reset();
	transmit_register_reset();

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(7800);
	set_tra_rate(7800);

	reset_key_state();
	start_processing(attotime::from_hz(7800));
}

//-------------------------------------------------
//  tra_callback - send bit to host
//-------------------------------------------------

void apricot_keyboard_hle_device::tra_callback()
{
	m_host->in_w(transmit_register_get_data_bit());
}

//-------------------------------------------------
//  received_byte - handle received byte
//-------------------------------------------------

void apricot_keyboard_hle_device::received_byte(uint8_t byte)
{
	if ((byte & 0xf0) == 0xf0)
	{
		// rtc data
		if (m_rtc_index >= 0)
		{
			m_rtc->address_w(m_rtc_index--);
			m_rtc->data_w(machine().dummy_space(), 0, byte);
			m_rtc->write_w(1);
			m_rtc->write_w(0);
		}
	}
	else
	{
		switch (byte)
		{
		case CMD_REQ_TIME_AND_DATE:
			logerror("System requests current time\n");

			// remove pending keys just in case
			clear_fifo();

			// send time prefix
			transmit_byte(0xed);

			// make rtc chip ready
			m_rtc->cs_w(1);
			m_rtc->read_w(1);

			// send bcd encoded date and time to system
			for (int i = 12; i >= 0; i--)
			{
				m_rtc->address_w(i);
				transmit_byte(0xf0 | m_rtc->data_r(machine().dummy_space(), 0));
			}

			break;

		case CMD_SET_TIME_AND_DATE:
			logerror("System requests to set time\n");

			// we start with the year
			m_rtc_index = 12;

			// make rtc chip ready
			m_rtc->cs_w(1);

			break;

		case CMD_ENABLE_MOUSE:
			logerror("System enables mouse\n");
			m_mouse_enabled = true;
			break;

		case CMD_DISABLE_MOUSE:
			logerror("System disables mouse\n");
			m_mouse_enabled = false;
			break;

		case CMD_KEYBOARD_RESET:
			logerror("System requests keyboard reset\n");
			transmit_byte(ACK_DIAGNOSTICS);
			break;

		default:
			logerror("Unhandled command: %02x\n", byte);
		}
	}
}

//-------------------------------------------------
//  key_make - handle a key being pressed
//-------------------------------------------------

void apricot_keyboard_hle_device::key_make(uint8_t row, uint8_t column)
{
	// send the make code
	transmit_byte((row << 3) | column);
}

//-------------------------------------------------
//  key_break - handle a key being released
//-------------------------------------------------

void apricot_keyboard_hle_device::key_break(uint8_t row, uint8_t column)
{
	// send the break code
	transmit_byte(0x80 | (row << 3) | column);
}

//-------------------------------------------------
//  out_w - receive bit from host
//-------------------------------------------------

void apricot_keyboard_hle_device::out_w(int state)
{
	device_buffered_serial_interface::rx_w(state);
}

//-------------------------------------------------
//  mouse_callback - check for new mouse events
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER( apricot_keyboard_hle_device::mouse_callback )
{
	if (m_mouse_enabled)
	{
		// get mouse state
		uint8_t buttons = m_mouse_b->read();
		uint8_t x = m_mouse_x->read();
		uint8_t y = m_mouse_y->read();

		// anything changed since last time?
		if (buttons != m_mouse_last_b || x != m_mouse_last_x || y != m_mouse_last_y)
		{
			// mouse header
			transmit_byte(0xef);

			// button state
			transmit_byte(0x70 | buttons);

			int8_t dx = x - m_mouse_last_x;
			int8_t dy = y - m_mouse_last_y;

			// y direction change
			transmit_byte(0x70 | ((dy >> 4) & 0x0f));
			transmit_byte(0x70 | ((dy >> 0) & 0x0f));

			// x direction change
			transmit_byte(0x70 | ((dx >> 4) & 0x0f));
			transmit_byte(0x70 | ((dx >> 0) & 0x0f));

			// save mouse state for next run
			m_mouse_last_b = buttons;
			m_mouse_last_x = x;
			m_mouse_last_y = y;
		}
	}
}
