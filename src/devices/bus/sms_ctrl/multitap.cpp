// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Furrtek's homemade multitap emulation

**********************************************************************/

#include "emu.h"
#include "multitap.h"


// Scheme: http://www.smspower.org/uploads/Homebrew/BOoM-SMS-sms4p_2.png


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_MULTITAP, sms_multitap_device, "sms_multitap", "Furrtek SMS Multitap")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_multitap_device - constructor
//-------------------------------------------------

sms_multitap_device::sms_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_MULTITAP, tag, owner, clock),
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
}


//-------------------------------------------------
//  sms_peripheral_r - multitap read
//-------------------------------------------------

uint8_t sms_multitap_device::peripheral_r()
{
	uint8_t data = 0xff;

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

void sms_multitap_device::peripheral_w(uint8_t data)
{
	uint8_t output_data;

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
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sms_multitap_device::device_add_mconfig(machine_config &config)
{
	// Controller subports setup, without the TH callback declaration,
	// because the circuit scheme shows TH of subports without connection.
	SMS_CONTROL_PORT(config, m_subctrl1_port, sms_control_port_devices, "joypad");
	SMS_CONTROL_PORT(config, m_subctrl2_port, sms_control_port_devices, "joypad");
	SMS_CONTROL_PORT(config, m_subctrl3_port, sms_control_port_devices, "joypad");
	SMS_CONTROL_PORT(config, m_subctrl4_port, sms_control_port_devices, "joypad");
}
