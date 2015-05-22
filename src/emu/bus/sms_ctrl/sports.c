// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Sports Pad" (US model) emulation

**********************************************************************/

#include "sports.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SMS_SPORTS_PAD = &device_creator<sms_sports_pad_device>;


#define SPORTS_PAD_INTERVAL attotime::from_hz(XTAL_53_693175MHz/15/512)


CUSTOM_INPUT_MEMBER( sms_sports_pad_device::dir_pins_r )
{
	UINT8 data = 0;

	switch (m_read_state)
	{
	case 0:
		data = m_sports_x->read() >> 4;
		break;
	case 1:
		data = m_sports_x->read();
		break;
	case 2:
		data = m_sports_y->read() >> 4;
		break;
	case 3:
		data = m_sports_y->read();
		break;
	}

	// The returned value is inverted due to IP_ACTIVE_LOW mapping.
	return ~(data & 0x0f);
}


CUSTOM_INPUT_MEMBER( sms_sports_pad_device::th_pin_r )
{
	return m_last_data;
}


INPUT_CHANGED_MEMBER( sms_sports_pad_device::th_pin_w )
{
	attotime cur_time = machine().time();

	if (cur_time - m_last_time > m_interval)
	{
		m_read_state = 0;
	}
	else
	{
		m_read_state = (m_read_state + 1) & 3;
	}
	m_last_time = cur_time;
	m_last_data = newval;
}


static INPUT_PORTS_START( sms_sports_pad )
	PORT_START("SPORTS_IN")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sms_sports_pad_device, dir_pins_r, NULL) // Directional pins
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // Vcc
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // TL (Button 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sms_sports_pad_device, th_pin_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) // TR (Button 2)

	PORT_START("SPORTS_OUT")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED ) // Directional pins
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // Vcc
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // TL (Button 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_CHANGED_MEMBER(DEVICE_SELF, sms_sports_pad_device, th_pin_w, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // TR (Button 2)

	PORT_START("SPORTS_X")    /* Sports Pad X axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_RESET PORT_REVERSE

	PORT_START("SPORTS_Y")    /* Sports Pad Y axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_RESET PORT_REVERSE
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sms_sports_pad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sms_sports_pad );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_sports_pad_device - constructor
//-------------------------------------------------

sms_sports_pad_device::sms_sports_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SMS_SPORTS_PAD, "Sega SMS Sports Pad US", tag, owner, clock, "sms_sports_pad", __FILE__),
	device_sms_control_port_interface(mconfig, *this),
	m_sports_in(*this, "SPORTS_IN"),
	m_sports_out(*this, "SPORTS_OUT"),
	m_sports_x(*this, "SPORTS_X"),
	m_sports_y(*this, "SPORTS_Y"),
	m_read_state(0),
	m_last_data(0),
	m_interval(SPORTS_PAD_INTERVAL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_sports_pad_device::device_start()
{
	m_last_time = machine().time();

	save_item(NAME(m_read_state));
	save_item(NAME(m_last_data));
	save_item(NAME(m_last_time));
}


//-------------------------------------------------
//  sms_peripheral_r - sports pad read
//-------------------------------------------------

UINT8 sms_sports_pad_device::peripheral_r()
{
	return m_sports_in->read();
}


//-------------------------------------------------
//  sms_peripheral_w - sports pad write
//-------------------------------------------------

void sms_sports_pad_device::peripheral_w(UINT8 data)
{
	m_sports_out->write(data);
}
