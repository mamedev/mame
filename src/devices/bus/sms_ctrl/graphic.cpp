// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Graphic Board" emulation

I/O 3f write | this method
        0x20 | 0x7f
        0x00 | 0x3f
        0x30 | 0xff

Typical sequence:
- 3f write 0x20
- read dc
- 3f write 0x00
- read dc
- 3f write 0x20
- read dc
- 3f write 0x30
Suspect from kind of counter that is reset by a 0x30 write to I/O port 0x3f.
Once reset reads from i/O port dc expect to see 0xE0.
And then any write with differing bits goes through several internal I/O ports
with the first port being the one with the buttons

In the reset/start state the lower four/five bits are 0.
Then a nibble is read containing the buttons (active low)
Then 2 nibbles are read to form a byte (first high nibble, then low nibble) indicating
whether the pen is on the graphic board, a value of FD, FE, or FF used for this. For
any other value the following 2 bytes are not read.
Then 2 nibbles are read to form a byte containing the absolute X coordinate.
THen 2 nibbles are read to form a byte containing the absolute Y coordiante.

**********************************************************************/

#include "graphic.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SMS_GRAPHIC = &device_creator<sms_graphic_device>;


static INPUT_PORTS_START( sms_graphic )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // MENU
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // DO
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // PEN
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(15)

	PORT_START("Y")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(15)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sms_graphic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sms_graphic );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_graphic_device - constructor
//-------------------------------------------------

sms_graphic_device::sms_graphic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SMS_GRAPHIC, "Sega SMS Graphic Board", tag, owner, clock, "sms_graphic", __FILE__)
	, device_sms_control_port_interface(mconfig, *this)
	, m_buttons(*this, "BUTTONS")
	, m_x(*this, "X")
	, m_y(*this, "Y")
	, m_index(0)
	, m_previous_write(0xff)
	, m_pressure(0xfd)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_graphic_device::device_start()
{
	save_item(NAME(m_index));
	save_item(NAME(m_previous_write));
}


//-------------------------------------------------
//  sms_peripheral_r - joypad read
//-------------------------------------------------

UINT8 sms_graphic_device::peripheral_r()
{
	switch (m_index)
	{
		case 0:     // Initial state / "I am a board"
			// If any regular button is pressed raise/lower TL ?
//          if ((m_buttons->read() & 0x07) != 0x07)
//              return 0xf0;
			return 0xd0;

		case 1:    // Read buttons (active low)
			return m_buttons->read();

		case 2:    // Some thing only FD, FE, and FF cause the other values to be read
			return m_pressure >> 4;

		case 3:
			return m_pressure & 0x0f;

		case 4:    // High nibble X?
			return m_x->read() >> 4;

		case 5:    // Low nibble X?
			return m_x->read() & 0x0f;

		case 6:   // High nibble Y?
			return m_y->read() >> 4;

		case 7:   // Low Nibble Y?
			return m_y->read() & 0x0f;
	}

	return 0xff;
}

void sms_graphic_device::peripheral_w(UINT8 data)
{
	// Check for toggle on TH/TL
	if ((data ^ m_previous_write) & 0xc0)
	{
		m_index++;
	}

	// If TR is high, restart
	if (data & 0x80)
	{
		m_index = 0;
	}

	m_previous_write = data;
}
