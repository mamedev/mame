// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn  multitap emulation

**********************************************************************/

#include "multitap.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SATURN_MULTITAP = &device_creator<saturn_multitap_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saturn_multitap_device - constructor
//-------------------------------------------------

saturn_multitap_device::saturn_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SATURN_MULTITAP, "Sega Saturn Multitap", tag, owner, clock, "saturn_multitap", __FILE__),
	device_saturn_control_port_interface(mconfig, *this),
	m_subctrl1_port(*this, "ctrl1"),
	m_subctrl2_port(*this, "ctrl2"),
	m_subctrl3_port(*this, "ctrl3"),
	m_subctrl4_port(*this, "ctrl4"),
	m_subctrl5_port(*this, "ctrl5"),
	m_subctrl6_port(*this, "ctrl6")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saturn_multitap_device::device_start()
{
	m_subctrl1_port->device_start();
	m_subctrl2_port->device_start();
	m_subctrl3_port->device_start();
	m_subctrl4_port->device_start();
	m_subctrl5_port->device_start();
	m_subctrl6_port->device_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void saturn_multitap_device::device_reset()
{
}

//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

UINT8 saturn_multitap_device::read_ctrl(UINT8 offset)
{
	UINT8 res = 0;
	switch (offset)
	{
		default:
		case 0:
		case 1:
			res = m_subctrl1_port->read_ctrl(offset & 1);
			break;
		case 2:
		case 3:
			res = m_subctrl2_port->read_ctrl(offset & 1);
			break;
		case 4:
		case 5:
			res = m_subctrl3_port->read_ctrl(offset & 1);
			break;
		case 6:
		case 7:
			res = m_subctrl4_port->read_ctrl(offset & 1);
			break;
		case 8:
		case 9:
			res = m_subctrl5_port->read_ctrl(offset & 1);
			break;
		case 10:
		case 11:
			res = m_subctrl6_port->read_ctrl(offset & 1);
			break;
	}
	return res;
}

//-------------------------------------------------
//  read_id
//-------------------------------------------------

UINT8 saturn_multitap_device::read_id(int idx)
{
	switch (idx)
	{
		case 0:
		default:
			return m_subctrl1_port->read_id(0);
		case 1:
			return m_subctrl2_port->read_id(0);
		case 2:
			return m_subctrl3_port->read_id(0);
		case 3:
			return m_subctrl4_port->read_id(0);
		case 4:
			return m_subctrl5_port->read_id(0);
		case 5:
			return m_subctrl6_port->read_id(0);
	}
}


static MACHINE_CONFIG_FRAGMENT( multitap_slot )
	MCFG_SATURN_CONTROL_PORT_ADD("ctrl1", saturn_joys, "joypad")
	MCFG_SATURN_CONTROL_PORT_ADD("ctrl2", saturn_joys, "joypad")
	MCFG_SATURN_CONTROL_PORT_ADD("ctrl3", saturn_joys, "joypad")
	MCFG_SATURN_CONTROL_PORT_ADD("ctrl4", saturn_joys, "joypad")
	MCFG_SATURN_CONTROL_PORT_ADD("ctrl5", saturn_joys, "joypad")
	MCFG_SATURN_CONTROL_PORT_ADD("ctrl6", saturn_joys, "joypad")
MACHINE_CONFIG_END


machine_config_constructor saturn_multitap_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( multitap_slot );
}
