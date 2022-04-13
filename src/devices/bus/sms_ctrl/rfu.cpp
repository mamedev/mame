// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega SG-1000/Mark-III/SMS "Rapid Fire Unit" emulation


Release data from the Sega Retro project:

  Year: 1985    Country/region: JP    Model code: RF-150
  Year: 1987    Country/region: US    Model code: 3046
  Year: 1988    Country/region: EU    Model code: MK-3046-50
  Year: 1989    Country/region: BR    Model code: 011050

Notes:

  This emulated device is the version released by Sega. In Brazil, Tec Toy
  released a version that does not have any switch to turn on/off auto-repeat.

**********************************************************************/

#include "emu.h"
#include "rfu.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_RAPID_FIRE, sms_rapid_fire_device, "sms_rapid_fire", "Sega SMS Rapid Fire Unit")

// time interval not verified
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

sms_rapid_fire_device::sms_rapid_fire_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_RAPID_FIRE, tag, owner, clock),
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
}


//-------------------------------------------------
//  sms_peripheral_r - rapid fire read
//-------------------------------------------------

uint8_t sms_rapid_fire_device::peripheral_r()
{
	uint8_t data;

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

void sms_rapid_fire_device::peripheral_w(uint8_t data)
{
	m_subctrl_port->port_w(data);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

WRITE_LINE_MEMBER( sms_rapid_fire_device::th_pin_w )
{
	m_port->th_pin_w(state);
}


void sms_rapid_fire_device::device_add_mconfig(machine_config &config)
{
	SMS_CONTROL_PORT(config, m_subctrl_port, sms_control_port_devices, "joypad");
	if (m_port != nullptr)
		m_subctrl_port->set_screen_tag(m_port->m_screen);
	m_subctrl_port->th_input_handler().set(FUNC(sms_rapid_fire_device::th_pin_w));
}
