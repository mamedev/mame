// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Rapid Fire Unit" emulation

**********************************************************************/

#include "rfu.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SMS_RAPID_FIRE = &device_creator<sms_rapid_fire_device>;


#define RAPID_FIRE_INTERVAL attotime::from_hz(10)


static INPUT_PORTS_START( sms_rapid_fire )
	PORT_START("rfu_sw")   // Rapid Fire Unit switches
	PORT_CONFNAME( 0x03, 0x00, "Rapid Fire Unit" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, "Button 1" )
	PORT_CONFSETTING( 0x02, "Button 2" )
	PORT_CONFSETTING( 0x03, "Button 1 + 2" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sms_rapid_fire_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sms_rapid_fire );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_rapid_fire_device - constructor
//-------------------------------------------------

sms_rapid_fire_device::sms_rapid_fire_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SMS_RAPID_FIRE, "Sega SMS Rapid Fire", tag, owner, clock, "sms_rapid_fire", __FILE__),
	device_sms_control_port_interface(mconfig, *this),
	m_rfire_sw(*this, "rfu_sw"),
	m_subctrl_port(*this, "ctrl"),
	m_read_state(0),
	m_interval(RAPID_FIRE_INTERVAL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_rapid_fire_device::device_start()
{
	m_start_time = machine().time();

	save_item(NAME(m_start_time));
	save_item(NAME(m_read_state));

	m_subctrl_port->device_start();
}


//-------------------------------------------------
//  sms_peripheral_r - rapid fire read
//-------------------------------------------------

UINT8 sms_rapid_fire_device::peripheral_r()
{
	UINT8 data;

	int num_intervals = (machine().time() - m_start_time).as_double() / m_interval.as_double();
	m_read_state = num_intervals & 1;

	data = m_subctrl_port->port_r();

	/* Check Rapid Fire switch for Button 1 (TL) */
	if (!(data & 0x20) && (m_rfire_sw->read() & 0x01))
		data |= m_read_state << 5;

	/* Check Rapid Fire switch for Button 2 (TR) */
	if (!(data & 0x80) && (m_rfire_sw->read() & 0x02))
		data |= m_read_state << 7;

	return data;
}


//-------------------------------------------------
//  sms_peripheral_w - rapid fire write
//-------------------------------------------------

void sms_rapid_fire_device::peripheral_w(UINT8 data)
{
	m_subctrl_port->port_w(data);
}


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

WRITE_LINE_MEMBER( sms_rapid_fire_device::th_pin_w )
{
	m_port->th_pin_w(state);
}


READ32_MEMBER( sms_rapid_fire_device::pixel_r )
{
	return m_port->pixel_r();
}


static MACHINE_CONFIG_FRAGMENT( rfire_slot )
	MCFG_SMS_CONTROL_PORT_ADD("ctrl", sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(WRITELINE(sms_rapid_fire_device, th_pin_w))
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_rapid_fire_device, pixel_r))
MACHINE_CONFIG_END


machine_config_constructor sms_rapid_fire_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( rfire_slot );
}
