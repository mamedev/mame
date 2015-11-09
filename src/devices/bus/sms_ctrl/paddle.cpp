// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Paddle Control" emulation

**********************************************************************/

#include "paddle.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SMS_PADDLE = &device_creator<sms_paddle_device>;


#define PADDLE_INTERVAL attotime::from_hz(XTAL_53_693175MHz/15/256)


CUSTOM_INPUT_MEMBER( sms_paddle_device::dir_pins_r )
{
	UINT8 data = m_paddle_x->read();

	if (m_read_state)
		data >>= 4;

	// The returned value is inverted due to IP_ACTIVE_LOW mapping.
	return ~data;
}


CUSTOM_INPUT_MEMBER( sms_paddle_device::tr_pin_r )
{
	// The returned value is inverted due to IP_ACTIVE_LOW mapping.
	return ~m_read_state;
}


static INPUT_PORTS_START( sms_paddle )
	PORT_START("CTRL_PORT")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sms_paddle_device, dir_pins_r, NULL) // Directional pins
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // Vcc
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // TL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // TH
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sms_paddle_device, tr_pin_r, NULL)

	PORT_START("PADDLE_X") // Paddle knob
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_SENSITIVITY(40) PORT_KEYDELTA(20) PORT_CENTERDELTA(0) PORT_MINMAX(0,255)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sms_paddle_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sms_paddle );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_paddle_device - constructor
//-------------------------------------------------

sms_paddle_device::sms_paddle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SMS_PADDLE, "Sega SMS Paddle", tag, owner, clock, "sms_paddle", __FILE__),
	device_sms_control_port_interface(mconfig, *this),
	m_paddle_pins(*this, "CTRL_PORT"),
	m_paddle_x(*this, "PADDLE_X"),
	m_read_state(0),
	m_interval(PADDLE_INTERVAL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_paddle_device::device_start()
{
	m_start_time = machine().time();

	save_item(NAME(m_start_time));
	save_item(NAME(m_read_state));
}


//-------------------------------------------------
//  sms_peripheral_r - paddle read
//-------------------------------------------------

UINT8 sms_paddle_device::peripheral_r()
{
	int num_intervals = (machine().time() - m_start_time).as_double() / m_interval.as_double();
	m_read_state = num_intervals & 1;

	return m_paddle_pins->read();
}
