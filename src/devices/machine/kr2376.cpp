// license:BSD-3-Clause
// copyright-holders:Roberto Lavarone
/**********************************************************************

    SMC KR2376 Keyboard Encoder emulation

**********************************************************************/

#include "emu.h"
#include "kr2376.h"

static const UINT8 KR2376_KEY_CODES[3][8][11] =
{
	// normal
	{
		//  Y0    Y1    Y2    Y3    Y4    Y5    Y6    Y7    Y8    Y9   Y10
		// NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   DC1    P     O        X0
		{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x11, 0x50, 0x30 }, // X0
		// DLE   K     L     N     M     NAK   SYN   ETB   CAN   EM    SUB       X1
		{ 0x10, 0x4b, 0x4c, 0x4e, 0x4d, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a }, // X1
		// -     FS    GS    RS    US    <     >     ,     SP    .     _         X2
		{ 0x2d, 0x1c, 0x1d, 0x1e, 0x1f, 0x3c, 0x3e, 0x2c, 0x20, 0x2e, 0x5f }, // X2
		// 0     :     p     _     @     BS    [     ]     CR    LF    DEL       X3
		{ 0x30, 0x3a, 0x70, 0x5f, 0x40, 0x08, 0x5B, 0x5d, 0x0d, 0x0a, 0x7f }, // X3
		{ 0x3b, 0x2f, 0x2e, 0x2c, 0x6d, 0x6e, 0x62, 0x76, 0x63, 0x78, 0x7a }, // X4
		{ 0x6c, 0x6b, 0x6a, 0x68, 0x67, 0x66, 0x64, 0x73, 0x61, 0x0c, 0x1b }, // X5
		{ 0x6f, 0x69, 0x75, 0x79, 0x74, 0x72, 0x65, 0x77, 0x71, 0x09, 0x0b }, // X6
		{ 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x5e, 0x5c }  // X7
	},

	// shift
	{
		//  Y0    Y1    Y2    Y3    Y4    Y5    Y6    Y7    Y8    Y9   Y10
		// NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   DC1    @     _        X0
		{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x11, 0x40, 0x5f }, // X0
		// DLE   [     \     ^     ]     NAK   SYN   ETB   CAN   EM    SUB       X1
		{ 0x10, 0x5b, 0x5c, 0x5e, 0x5d, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a }, // X1
		// =     FS    GS    RS    US    <     >     ,     SP    .     _         X2
		{ 0x3d, 0x1c, 0x1d, 0x1e, 0x1f, 0x3c, 0x3e, 0x2c, 0x20, 0x2e, 0x5f }, // X2
		// NUL   *     P     DEL   `     BS    {     }     CR    LF    DEL       X3
		{ 0x00, 0x2a, 0x50, 0x7f, 0x60, 0x08, 0x7b, 0x7d, 0x0d, 0x0a, 0x7f }, // X3
		{ 0x2b, 0x3f, 0x3e, 0x3c, 0x4d, 0x4e, 0x42, 0x56, 0x43, 0x58, 0x5a }, // X4
		{ 0x4c, 0x4b, 0x4a, 0x48, 0x47, 0x46, 0x44, 0x53, 0x41, 0x0c, 0x1b }, // X5
		{ 0x4f, 0x49, 0x55, 0x59, 0x54, 0x52, 0x45, 0x57, 0x51, 0x09, 0x0b }, // X6
		{ 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x7e, 0x7c }  // X7
	},

	// control
	{
		//  Y0    Y1    Y2    Y3    Y4    Y5    Y6    Y7    Y8    Y9   Y10
		// NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   DC1   DLE   SI        X0
		{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x11, 0x10, 0x0f }, // X0
		// DLE   VT    FF    SO    CR    NAK   SYN   ETB   CAN   EM    SUB       X1
		{ 0x10, 0x0b, 0x0c, 0x0e, 0x0d, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a }, // X1
		// NUL   FS    GS    RS    US    NUL   NUL   NUL   SP    NUL   US        X2
		{ 0x00, 0x1c, 0x1d, 0x1e, 0x1f, 0x00, 0x00, 0x00, 0x20, 0x00, 0x1f }, // X2
		// NUL   NUL   DLE   US    NUL   BS    ESC   GS    CR    LF    DEL       X3
		{ 0x00, 0x00, 0x10, 0x1f, 0x00, 0x08, 0x1B, 0x1d, 0x0d, 0x0a, 0x7f }, // X3
		{ 0x00, 0x00, 0x00, 0x00, 0x1d, 0x0e, 0x02, 0x16, 0x03, 0x18, 0x1a }, // X4
		{ 0x0c, 0x0b, 0x0a, 0x08, 0x07, 0x06, 0x04, 0x13, 0x01, 0x0c, 0x1b }, // X5
		{ 0x1f, 0x09, 0x15, 0x19, 0x14, 0x12, 0x05, 0x17, 0x11, 0x09, 0x0b }, // X6
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x1c }  // X7
	}
};


const device_type KR2376 = &device_creator<kr2376_device>;

kr2376_device::kr2376_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KR2376, "SMC KR2376", tag, owner, clock, "kr2376", __FILE__),
	m_write_strobe(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kr2376_device::device_start()
{
	m_write_strobe.resolve_safe();

	/* set initial values */
	m_ring11 = 0;
	m_ring8 = 0;
	m_modifiers = 0;
	m_strobe = 0;
	m_strobe_old = 0;
	m_parity = 0;
	m_data = 0;
	memset(m_pins, 0x00, sizeof(m_pins));
	change_output_lines();

	/* create the timers */
	m_scan_timer = timer_alloc(TIMER_SCAN_TICK);
	m_scan_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));

	/* register for state saving */
	save_item(NAME(m_pins));
	save_item(NAME(m_ring11));
	save_item(NAME(m_ring8));
	save_item(NAME(m_modifiers));
	save_item(NAME(m_strobe));
	save_item(NAME(m_strobe_old));
	save_item(NAME(m_parity));
	save_item(NAME(m_data));
}

/*-------------------------------------------------
    set_input_pin - set an input pin
-------------------------------------------------*/
void kr2376_device::set_input_pin( kr2376_input_pin_t pin, int data )
{
	data = data ? 1 : 0;
	switch ( pin )
	{
	case KR2376_PII:
	case KR2376_DSII:
		m_pins[pin] = data;
		break;
	}
}


/*-------------------------------------------------
    get_output_pin - get the status of an output pin
-------------------------------------------------*/
int kr2376_device::get_output_pin( kr2376_output_pin_t pin )
{
	return m_pins[pin];
}


void kr2376_device::change_output_lines()
{
	if (m_strobe != m_strobe_old)
	{
		m_strobe_old = m_strobe;

		if (m_strobe) // strobe 0 --> 1 transition
		{
			/* update parity */
			m_pins[KR2376_PO] = m_parity ^ m_pins[KR2376_PII];
		}
		m_pins[KR2376_SO] = m_strobe ^ m_pins[KR2376_DSII];
		m_write_strobe(m_strobe ^ m_pins[KR2376_DSII]);
	}
}

void kr2376_device::clock_scan_counters()
{
	/* ring counters inhibited while strobe active */
	if (!m_strobe)
	{
		m_ring11++;
		if (m_ring11 == 11)
		{
			m_ring11 = 0;
			m_ring8++;
			if (m_ring8 == 8)
				m_ring8 = 0;
		}
	}
}

void kr2376_device::detect_keypress()
{
	static const char *const keynames[] = { "X0", "X1", "X2", "X3", "X4", "X5", "X6", "X7" };

	if (ioport(keynames[m_ring8])->read() == (1 << m_ring11))
	{
		m_modifiers = ioport("MODIFIERS")->read();

		m_strobe = 1;
		/*  strobe 0->1 transition, encode char and update parity */
		if (!m_strobe_old)
		{
			int i;
			int parbit;
			int shift = BIT(m_modifiers, 0);
			int control = BIT(m_modifiers, 1);
			int alpha = BIT(m_modifiers, 2);
			int table = 0;

			if (shift || alpha)
				table = 1;
			else if (control)
				table = 2;

			m_data = KR2376_KEY_CODES[table][m_ring8][m_ring11];

			/* Compute ODD parity */
			m_parity = m_data;
			parbit = 0;
			for (i=0; i<8; i++)
				parbit ^= (m_parity >> i) & 1;
			m_parity = parbit;
		}
	}
	else
	{
		m_strobe = 0;
	}
}

void kr2376_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
		{
			case TIMER_SCAN_TICK:
				change_output_lines();
				clock_scan_counters();
				detect_keypress();
			break;
		}
}

/* Keyboard Data */

READ8_MEMBER( kr2376_device::data_r )
{
	if (m_pins[KR2376_DSII])
		return m_data ^ 0xff;
	else
		return m_data;
}

/* Input Ports */

INPUT_PORTS_START( kr2376 )
	PORT_START("X0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )


	PORT_START("X2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )                               PORT_CHAR('_')

	PORT_START("X3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('`') PORT_CHAR('@')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)                   PORT_NAME("Del")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                  PORT_NAME("CR")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(10) PORT_NAME("LF")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')


	PORT_START("X5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)          PORT_CHAR('c') PORT_CHAR('G')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))


	PORT_START("X6")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("X7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')

	PORT_START("MODIFIERS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor kr2376_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( kr2376 );
}
