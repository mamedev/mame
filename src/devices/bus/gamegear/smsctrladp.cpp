// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Game Gear "SMS Controller Adaptor" emulation
    Also known as "Master Link" cable.

**********************************************************************/

#include "smsctrladp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SMS_CTRL_ADAPTOR = &device_creator<sms_ctrl_adaptor_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_ctrl_adaptor_device - constructor
//-------------------------------------------------

sms_ctrl_adaptor_device::sms_ctrl_adaptor_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SMS_CTRL_ADAPTOR, "SMS Controller Adaptor", tag, owner, clock, "sms_ctrl_adaptor", __FILE__),
	device_gg_ext_port_interface(mconfig, *this),
	m_subctrl_port(*this, "ctrl")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_ctrl_adaptor_device::device_start()
{
	m_subctrl_port->device_start();
}


//-------------------------------------------------
//  sms_peripheral_r - rapid fire read
//-------------------------------------------------

UINT8 sms_ctrl_adaptor_device::peripheral_r()
{
	return m_subctrl_port->port_r();
}


//-------------------------------------------------
//  sms_peripheral_w - rapid fire write
//-------------------------------------------------

void sms_ctrl_adaptor_device::peripheral_w(UINT8 data)
{
	m_subctrl_port->port_w(data);
}


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

WRITE_LINE_MEMBER( sms_ctrl_adaptor_device::th_pin_w )
{
	m_port->th_pin_w(state);
}


READ32_MEMBER( sms_ctrl_adaptor_device::pixel_r )
{
	return m_port->pixel_r();
}


static MACHINE_CONFIG_FRAGMENT( sms_adp_slot )
	MCFG_SMS_CONTROL_PORT_ADD("ctrl", sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_TH_INPUT_HANDLER(WRITELINE(sms_ctrl_adaptor_device, th_pin_w))
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_ctrl_adaptor_device, pixel_r))
MACHINE_CONFIG_END


machine_config_constructor sms_ctrl_adaptor_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sms_adp_slot );
}
