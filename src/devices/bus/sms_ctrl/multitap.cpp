// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Furrtek's homemade multitap emulation

**********************************************************************/

#include "multitap.h"


// Scheme: http://www.smspower.org/uploads/Homebrew/BOoM-SMS-sms4p_2.png


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SMS_MULTITAP = &device_creator<sms_multitap_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_multitap_device - constructor
//-------------------------------------------------

sms_multitap_device::sms_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SMS_MULTITAP, "Sega SMS Multitap", tag, owner, clock, "sms_multitap", __FILE__),
	device_sms_control_port_interface(mconfig, *this),
	m_subctrl1_port(*this, "ctrl1"),
	m_subctrl2_port(*this, "ctrl2"),
	m_subctrl3_port(*this, "ctrl3"),
	m_subctrl4_port(*this, "ctrl4"),
	m_read_state(0),
	m_last_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_multitap_device::device_start()
{
	save_item(NAME(m_read_state));
	save_item(NAME(m_last_data));

	m_subctrl1_port->device_start();
	m_subctrl2_port->device_start();
	m_subctrl3_port->device_start();
	m_subctrl4_port->device_start();
}


//-------------------------------------------------
//  sms_peripheral_r - multitap read
//-------------------------------------------------

UINT8 sms_multitap_device::peripheral_r()
{
	UINT8 data = 0xff;

	switch(m_read_state)
	{
	case 0:
		data = m_subctrl1_port->port_r();
		break;
	case 1:
		data = m_subctrl2_port->port_r();
		break;
	case 2:
		data = m_subctrl3_port->port_r();
		break;
	case 3:
		data = m_subctrl4_port->port_r();
		break;
	}

	// force TH level high (1), as the line is not connected to subports.
	data |= 0x40;

	return data;
}


//-------------------------------------------------
//  sms_peripheral_w - multitap write
//-------------------------------------------------

void sms_multitap_device::peripheral_w(UINT8 data)
{
	UINT8 output_data;

	// check if TH level is low (0) and was high (1)
	if (!(data & 0x40) && (m_last_data & 0x40))
	{
		m_read_state = (m_read_state + 1) & 3;
	}
	m_last_data = data;

	// output default TH level (1), as the line is not connected to subports.
	output_data = data | 0x40;

	switch(m_read_state)
	{
	case 0:
		m_subctrl1_port->port_w(output_data);
		break;
	case 1:
		m_subctrl2_port->port_w(output_data);
		break;
	case 2:
		m_subctrl3_port->port_w(output_data);
		break;
	case 3:
		m_subctrl4_port->port_w(output_data);
		break;
	}
}


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

READ32_MEMBER( sms_multitap_device::pixel_r )
{
	return m_port->pixel_r();
}


static MACHINE_CONFIG_FRAGMENT( multitap_slot )
	// Controller subports setup, without the TH callback declaration,
	// because the circuit scheme shows TH of subports without connection.
	MCFG_SMS_CONTROL_PORT_ADD("ctrl1", sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_multitap_device, pixel_r))
	MCFG_SMS_CONTROL_PORT_ADD("ctrl2", sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_multitap_device, pixel_r))
	MCFG_SMS_CONTROL_PORT_ADD("ctrl3", sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_multitap_device, pixel_r))
	MCFG_SMS_CONTROL_PORT_ADD("ctrl4", sms_control_port_devices, "joypad")
	MCFG_SMS_CONTROL_PORT_PIXEL_HANDLER(READ32(sms_multitap_device, pixel_r))
MACHINE_CONFIG_END


machine_config_constructor sms_multitap_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( multitap_slot );
}
