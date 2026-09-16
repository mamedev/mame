// license:BSD-3-Clause
// copyright-holders:Roberto Lavarone
/**********************************************************************

    SMC KR2376 Keyboard Encoder emulation

    Notes:
    - the ROM contains 2376 bits, 3 x 88 x 9 bits per code. The last bit is
      parity which we calculate so only define the 8 data bits.

**********************************************************************/

#include "emu.h"
#include "kr2376.h"


DEFINE_DEVICE_TYPE(KR2376_ST,  kr2376_st_device,  "kr2376_st",  "SMC KR2376-ST Keyboard Encoder")
DEFINE_DEVICE_TYPE(KR2376_12,  kr2376_12_device,  "kr2376_12",  "SMC KR2376-12 Keyboard Encoder")

kr2376_device::kr2376_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_read_x(*this, 0x7ff),
	m_read_shift(*this, 0),
	m_read_control(*this, 0),
	m_write_strobe(*this)
{
}

kr2376_st_device::kr2376_st_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kr2376_device(mconfig, KR2376_ST, tag, owner, clock)
{
}

uint8_t kr2376_st_device::key_codes(int mode, int x, int y)
{
	static const uint8_t KEY_CODES[3][8][11] =
	{
		// normal
		{
			//  Y0    Y1    Y2    Y3    Y4    Y5    Y6    Y7    Y8    Y9   Y10
			// NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   DC1    P     O        X0
			{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x11, 0x50, 0x30 }, // X0
			// DLE   K     L     N     M     NAK   SYN   ETB   CAN   EM    SUB       X1
			{ 0x10, 0x4b, 0x4c, 0x4e, 0x4d, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a }, // X1
			// -     FS    GS    RS    US    <     >     ,     SP    .     _         X2
			{ 0xad, 0x1c, 0x1d, 0x1e, 0x1f, 0xbc, 0xbe, 0xac, 0xa0, 0xae, 0x5f }, // X2
			// 0     :     p     _     @     BS    [     ]     CR    LF    DEL       X3
			{ 0xb0, 0xba, 0x70, 0x5f, 0x40, 0x08, 0x5B, 0x5d, 0x0d, 0x0a, 0xff }, // X3
			{ 0xbb, 0xaf, 0xae, 0xac, 0x6d, 0x6e, 0x62, 0x76, 0x63, 0x78, 0x7a }, // X4
			{ 0x6c, 0x6b, 0x6a, 0x68, 0x67, 0x66, 0x64, 0x73, 0x61, 0x0c, 0x1b }, // X5
			{ 0x6f, 0x69, 0x75, 0x79, 0x74, 0x72, 0x65, 0x77, 0x71, 0x09, 0x0b }, // X6
			{ 0xb9, 0xb8, 0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0x5e, 0x5c }  // X7
		},

		// shift
		{
			//  Y0    Y1    Y2    Y3    Y4    Y5    Y6    Y7    Y8    Y9   Y10
			// NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   DC1    @     _        X0
			{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x11, 0x40, 0x5f }, // X0
			// DLE   [     \     ^     ]     NAK   SYN   ETB   CAN   EM    SUB       X1
			{ 0x10, 0x5b, 0x5c, 0x5e, 0x5d, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a }, // X1
			// =     FS    GS    RS    US    <     >     ,     SP    .     _         X2
			{ 0xbd, 0x1c, 0x1d, 0x1e, 0x1f, 0xbc, 0xbe, 0xac, 0xa0, 0xae, 0x5f }, // X2
			// NUL   *     P     DEL   `     BS    {     }     CR    LF    DEL       X3
			{ 0x00, 0xaa, 0x50, 0xff, 0xe0, 0x08, 0xfb, 0xfd, 0x0d, 0x0a, 0xff }, // X3
			{ 0xab, 0xbf, 0xbe, 0xbc, 0x4d, 0x4e, 0x42, 0x56, 0x43, 0x58, 0x5a }, // X4
			{ 0x4c, 0x4b, 0x4a, 0x48, 0x47, 0x46, 0x44, 0x53, 0x41, 0x0c, 0x1b }, // X5
			{ 0x4f, 0x49, 0x55, 0x59, 0x54, 0x52, 0x45, 0x57, 0x51, 0x09, 0x0b }, // X6
			{ 0xa9, 0xa8, 0xa7, 0xa6, 0xa5, 0xa4, 0xa3, 0xa2, 0xa1, 0xfe, 0xfc }  // X7
		},

		// control
		{
			//  Y0    Y1    Y2    Y3    Y4    Y5    Y6    Y7    Y8    Y9   Y10
			// NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   DC1   DLE   SI        X0
			{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x11, 0x10, 0x0f }, // X0
			// DLE   VT    FF    SO    CR    NAK   SYN   ETB   CAN   EM    SUB       X1
			{ 0x10, 0x0b, 0x0c, 0x0e, 0x0d, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a }, // X1
			// NUL   FS    GS    RS    US    NUL   NUL   NUL   SP    NUL   US        X2
			{ 0x00, 0x1c, 0x1d, 0x1e, 0x1f, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x1f }, // X2
			// NUL   NUL   DLE   US    NUL   BS    ESC   GS    CR    LF    DEL       X3
			{ 0x00, 0x00, 0x10, 0x1f, 0x00, 0x08, 0x1B, 0x1d, 0x0d, 0x0a, 0xff }, // X3
			{ 0x00, 0x00, 0x00, 0x00, 0x1d, 0x0e, 0x02, 0x16, 0x03, 0x18, 0x1a }, // X4
			{ 0x0c, 0x0b, 0x0a, 0x08, 0x07, 0x06, 0x04, 0x13, 0x01, 0x0c, 0x1b }, // X5
			{ 0x1f, 0x09, 0x15, 0x19, 0x14, 0x12, 0x05, 0x17, 0x11, 0x09, 0x0b }, // X6
			{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x1c }  // X7
		}
	};
	return KEY_CODES[mode][x][y];
}

kr2376_12_device::kr2376_12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
  : kr2376_device(mconfig, KR2376_12, tag, owner, clock)
{
}

uint8_t kr2376_12_device::key_codes(int mode, int x, int y)
{
	static const uint8_t KEY_CODES[3][8][11] =
	{
		// normal
		{
			//  Y0    Y1    Y2    Y3    Y4    Y5    Y6    Y7    Y8    Y9   Y10
			// 3     2     1     RS    SUB   GS    SYN   STX   NAK   DC4   NUL
			{ 0x33, 0x32, 0x31, 0x1e, 0x1a, 0x1d, 0x16, 0x02, 0x15, 0x14, 0x00 }, // X0
			// 6     5     4     ]     [     ETX   CAN   GS    DEL   DLE   SOH
			{ 0x36, 0x35, 0x34, 0x5d, 0x5b, 0x03, 0x18, 0x1d, 0x7f, 0x10, 0x01 }, // X1
			// 9     8     7     |     ~     FS    EM    ESC   SUB   HT    ACK
			{ 0x39, 0x38, 0x37, 0x7c, 0x7e, 0x1c, 0x19, 0x1b, 0x1a, 0x09, 0x06 }, // X2
			// US    VT    DC1   BS    SI    HT    LF    FF    CR    SP    ETB
			{ 0x1f, 0x0b, 0x11, 0x08, 0x0f, 0x09, 0x0a, 0x0c, 0x0d, 0x20, 0x17 }, // X3
			// -     0     9     8     7     6     5     4     3     2     1
			{ 0x2d, 0x30, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31 }, // X4
			// \     p     o     i     u     y     t     r     e     w     q
			{ 0x5c, 0x70, 0x6f, 0x69, 0x75, 0x79, 0x74, 0x72, 0x65, 0x77, 0x71 }, // X5
			// :     ;     l     k     j     h     g     f     d     s     a
			{ 0x3a, 0x3b, 0x6c, 0x6b, 0x6a, 0x68, 0x67, 0x66, 0x64, 0x73, 0x61 }, // X6
			// _     /     .     ,     m     n     b     v     c     x     z
			{ 0x5f, 0x2f, 0x2e, 0x2c, 0x6d, 0x6e, 0x62, 0x76, 0x63, 0x78, 0x7a }  // X7
		},

		// shift
		{
			//  Y0    Y1    Y2    Y3    Y4    Y5    Y6    Y7    Y8    Y9   Y10
			// 3     2     1     RS    SUB   DC2   SYN   STX   NAK   DC4   NUL
			{ 0x33, 0x32, 0x31, 0x1e, 0x1a, 0x12, 0x16, 0x02, 0x15, 0x14, 0x00 }, // X0
			// 6     5     4     }     {     ETX   CAN   GS    DEL   DLE   SOH
			{ 0x36, 0x35, 0x34, 0x7d, 0x7b, 0x03, 0x18, 0x1d, 0x7f, 0x10, 0x01 }, // X1
			// 9     8     7     \     ^     FS    EM    ESC   SUB   HT    ACK
			{ 0x39, 0x38, 0x37, 0x5c, 0x5e, 0x1c, 0x19, 0x1b, 0x1a, 0x09, 0x06 }, // X2
			// US    VT    DC1   BS    SO    DC3   LF    FF    CR    SP    ETB
			{ 0x1f, 0x0b, 0x11, 0x08, 0x0e, 0x13, 0x0a, 0x0c, 0x0d, 0x20, 0x17 }, // X3
			// =     0     )     (     '     &     %     $     #     "     !
			{ 0x3d, 0x30, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21 }, // X4
			// @     P     O     I     U     Y     T     R     E     W     Q
			{ 0x40, 0x50, 0x4f, 0x49, 0x55, 0x59, 0x54, 0x52, 0x45, 0x57, 0x51 }, // X5
			// *     +     L     K     J     H     G     F     D     S     A
			{ 0x2a, 0x2b, 0x4c, 0x4b, 0x4a, 0x48, 0x47, 0x46, 0x44, 0x53, 0x41 }, // X6
			// _     ?     >     <     M     N     B     V     C     X     Z
			{ 0x5f, 0x3f, 0x3e, 0x3c, 0x4d, 0x4e, 0x42, 0x56, 0x43, 0x58, 0x5a }  // X7
		},

		// control
		{
			//  Y0    Y1    Y2    Y3    Y4    Y5    Y6    Y7    Y8    Y9   Y10
			// 3     2     1     RS    SUB   GS    SYN   STX   NAK   DC4   NUL
			{ 0x33, 0x32, 0x31, 0x1e, 0x1a, 0x1d, 0x16, 0x02, 0x15, 0x14, 0x00 }, // X0
			// 6     5     4     GS    ESC   ETX   CAN   GS    DEL   DLE   SOH
			{ 0x36, 0x35, 0x34, 0x1d, 0x1b, 0x03, 0x18, 0x1d, 0x7f, 0x10, 0x01 }, // X1
			// 9     8     7     FS    RS    FS    EM    ESC   SUB   HT    ACK
			{ 0x39, 0x38, 0x37, 0x1c, 0x1e, 0x1c, 0x19, 0x1b, 0x1a, 0x09, 0x06 }, // X2
			// US    VT    DC1   BS    SI    HT    LF    FF    CR    SP    ETB
			{ 0x1f, 0x0b, 0x11, 0x08, 0x0f, 0x09, 0x0a, 0x0c, 0x0d, 0x20, 0x17 }, // X3
			// -     0     9     8     7     6     5     4     3     2     1
			{ 0x2d, 0x30, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31 }, // X4
			// NUL   DLE   SI    HT    NAK   EM    DC4   DC2   ENQ   ETB   DC1
			{ 0x00, 0x10, 0x0f, 0x09, 0x15, 0x19, 0x14, 0x12, 0x05, 0x17, 0x11 }, // X5
			// :     ;     FF    VT    LF    BS    BEL   ACK   EOT   DC3   SOH
			{ 0x3a, 0x3b, 0x0c, 0x0b, 0x0a, 0x08, 0x07, 0x06, 0x04, 0x13, 0x01 }, // X6
			// US    /     .     ,     CR    SO    STX   SYN   ETX   CAN   SUB
			{ 0x1f, 0x2f, 0x2e, 0x2c, 0x0d, 0x0e, 0x02, 0x16, 0x03, 0x18, 0x1a }  // X7
		}
	};
	return KEY_CODES[mode][x][y];
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kr2376_device::device_start()
{
	/* set initial values */
	m_ring11 = 0;
	m_ring8 = 0;
	m_strobe = 0;
	m_strobe_old = 0;
	m_parity = 0;
	m_data = 0;
	memset(m_pins, 0x00, sizeof(m_pins));
	change_output_lines();

	/* create the timers */
	m_scan_timer = timer_alloc(FUNC(kr2376_device::perform_scan), this);
	m_scan_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));

	/* register for state saving */
	save_item(NAME(m_pins));
	save_item(NAME(m_ring11));
	save_item(NAME(m_ring8));
	save_item(NAME(m_strobe));
	save_item(NAME(m_strobe_old));
	save_item(NAME(m_parity));
	save_item(NAME(m_data));
}

/*-------------------------------------------------
    set_input_pin - set an input pin
-------------------------------------------------*/
void kr2376_device::set_input_pin( input_pin_t pin, int data )
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
int kr2376_device::get_output_pin( output_pin_t pin )
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
	if (m_read_x[m_ring8]() == (1 << m_ring11))
	{
		m_strobe = 1;
		/*  strobe 0->1 transition, encode char and update parity */
		if (!m_strobe_old)
		{
			int parbit;
			int table = 0;

			if (m_read_shift())
				table = 1;
			else if (m_read_control())
				table = 2;

			m_data = key_codes(table, m_ring8, m_ring11);

			/* Compute ODD parity */
			m_parity = m_data;
			parbit = 0;
			for (int i=0; i<8; i++)
				parbit ^= (m_parity >> i) & 1;
			m_parity = parbit;
		}
	}
	else
	{
		m_strobe = 0;
	}
}

TIMER_CALLBACK_MEMBER(kr2376_device::perform_scan)
{
	change_output_lines();
	clock_scan_counters();
	detect_keypress();
}

/* Keyboard Data */

uint8_t kr2376_device::data_r()
{
	if (m_pins[KR2376_DSII])
		return m_data ^ 0xff;
	else
		return m_data;
}
