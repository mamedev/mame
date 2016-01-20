// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Sports Pad" (Japanese model) emulation

**********************************************************************/

// The Japanese Sports Pad controller is only required to play the cartridge
// Sports Pad Soccer, released in Japan. It uses a different mode than the
// used by the US model, due to missing output lines on Sega Mark III
// controller ports.

#include "sportsjp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SMS_SPORTS_PAD_JP = &device_creator<sms_sports_pad_jp_device>;


#define SPORTS_PAD_JP_INTERVAL attotime::from_hz(30000) // 30Hz (not measured)


DECLARE_CUSTOM_INPUT_MEMBER( sms_sports_pad_jp_device::dir_pins_r )
{
	UINT8 data = 0;

	switch (m_read_state)
	{
	case 0:
		data = m_sports_jp_x->read() >> 4;
		break;
	case 1:
		data = m_sports_jp_x->read();
		break;
	case 2:
		data = m_sports_jp_y->read() >> 4;
		break;
	case 3:
		data = m_sports_jp_y->read();
		break;
	}

	// The returned value is inverted due to IP_ACTIVE_LOW mapping.
	return ~(data & 0x0f);
}


static INPUT_PORTS_START( sms_sports_pad_jp )
	PORT_START("SPORTS_JP_IN")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sms_sports_pad_jp_device, dir_pins_r, NULL) // Directional pins
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // Vcc
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // TL (Button 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // TH
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) // TR (Button 2)

	PORT_START("SPORTS_JP_X")    /* Sports Pad X axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40)

	PORT_START("SPORTS_JP_Y")    /* Sports Pad Y axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sms_sports_pad_jp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sms_sports_pad_jp );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_sports_pad_jp_device - constructor
//-------------------------------------------------

sms_sports_pad_jp_device::sms_sports_pad_jp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SMS_SPORTS_PAD_JP, "Sega SMS Sports Pad JP", tag, owner, clock, "sms_sports_pad_jp", __FILE__),
	device_sms_control_port_interface(mconfig, *this),
	m_sports_jp_in(*this, "SPORTS_JP_IN"),
	m_sports_jp_x(*this, "SPORTS_JP_X"),
	m_sports_jp_y(*this, "SPORTS_JP_Y"),
	m_read_state(0),
	m_interval(SPORTS_PAD_JP_INTERVAL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_sports_pad_jp_device::device_start()
{
	m_start_time = machine().time();

	save_item(NAME(m_start_time));
	save_item(NAME(m_read_state));
}


//-------------------------------------------------
//  sms_peripheral_r - sports pad read
//-------------------------------------------------

UINT8 sms_sports_pad_jp_device::peripheral_r()
{
	UINT8 data;
	int num_intervals = (machine().time() - m_start_time).as_double() / m_interval.as_double();
	m_read_state = num_intervals % 5;

	data = m_sports_jp_in->read();

	switch (m_read_state)
	{
	case 0:
		// X high nibble
		data &= ~0x20; // TL 0
		data &= ~0x80; // TR 0
		break;
	case 1:
		// X low nibble
		data |= 0x20;  // TL 1
		data &= ~0x80; // TR 0
		break;
	case 2:
		// Y high nibble
		data &= ~0x20; // TL 0
		data &= ~0x80; // TR 0
		break;
	case 3:
		// Y low nibble
		data |= 0x20;  // TL 1
		data &= ~0x80; // TR 0
		break;
	case 4:
		// buttons 1 and 2
		data = (data & 0x20) >> 5 | (data & 0x80) >> 6 | 0xfc;
		data |= 0x20; // TL 1
		data |= 0x80; // TR 1
		break;
	}

	return data;
}
