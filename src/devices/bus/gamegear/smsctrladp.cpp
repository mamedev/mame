// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Game Gear "SMS Controller Adaptor" emulation
    Also known as "Master Link" cable.

**********************************************************************/

#include "emu.h"
#include "smsctrladp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_CTRL_ADAPTOR, sms_ctrl_adaptor_device, "sms_ctrl_adaptor", "SMS Controller Adaptor")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_ctrl_adaptor_device - constructor
//-------------------------------------------------

sms_ctrl_adaptor_device::sms_ctrl_adaptor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_CTRL_ADAPTOR, tag, owner, clock),
	device_gg_ext_port_interface(mconfig, *this),
	m_subctrl_port(*this, "ctrl")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_ctrl_adaptor_device::device_start()
{
}


//-------------------------------------------------
//  sms_peripheral_r - sms_ctrl_adaptor read
//-------------------------------------------------

uint8_t sms_ctrl_adaptor_device::peripheral_r()
{
	return m_subctrl_port->port_r();
}


//-------------------------------------------------
//  sms_peripheral_w - sms_ctrl_adaptor write
//-------------------------------------------------

void sms_ctrl_adaptor_device::peripheral_w(uint8_t data)
{
	m_subctrl_port->port_w(data);
}


WRITE_LINE_MEMBER( sms_ctrl_adaptor_device::th_pin_w )
{
	m_port->th_pin_w(state);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sms_ctrl_adaptor_device::device_add_mconfig(machine_config &config)
{
	SMS_CONTROL_PORT(config, m_subctrl_port, sms_control_port_devices, "joypad");
	if (m_port != nullptr)
		m_subctrl_port->set_screen_tag(m_port->m_screen);
	m_subctrl_port->th_input_handler().set(FUNC(sms_ctrl_adaptor_device::th_pin_w));
}

