// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn SegaTap / Team Player emulation

**********************************************************************/

#include "emu.h"
#include "segatap.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SATURN_SEGATAP, saturn_segatap_device, "saturn_segatap", "saturn_segatap_device")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  saturn_segatap_device - constructor
//-------------------------------------------------

saturn_segatap_device::saturn_segatap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SATURN_SEGATAP, tag, owner, clock)
	, device_saturn_control_port_interface(mconfig, *this)
	, m_subctrl_port(*this, "ctrl%u", 1U)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void saturn_segatap_device::device_start()
{
	for (int i = 0; i < 4; i++)
		m_subctrl_port[i]->device_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void saturn_segatap_device::device_reset()
{
}

//-------------------------------------------------
//  read_ctrl
//-------------------------------------------------

uint8_t saturn_segatap_device::read_ctrl(uint8_t offset)
{
	return m_subctrl_port[offset < 8 ? (offset >> 1) : 0]->read_ctrl(offset & 1);
}

//-------------------------------------------------
//  read_id
//-------------------------------------------------

uint8_t saturn_segatap_device::read_id(int idx)
{
	return m_subctrl_port[idx < 4 ? idx : 0]->read_id(0);
}


void saturn_segatap_device::device_add_mconfig(machine_config &config)
{
	SATURN_CONTROL_PORT(config, m_subctrl_port[0], saturn_joys, "joypad");
	SATURN_CONTROL_PORT(config, m_subctrl_port[1], saturn_joys, "joypad");
	SATURN_CONTROL_PORT(config, m_subctrl_port[2], saturn_joys, "joypad");
	SATURN_CONTROL_PORT(config, m_subctrl_port[3], saturn_joys, "joypad");
}
