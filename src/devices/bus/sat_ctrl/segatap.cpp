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

saturn_segatap_device::saturn_segatap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SATURN_SEGATAP, tag, owner, clock)
	, device_saturn_control_port_interface(mconfig, *this)
	, m_subctrl_port(*this, "ctrl%u", 1U)
{
}


void saturn_segatap_device::device_start()
{
}

void saturn_segatap_device::device_add_mconfig(machine_config &config)
{
	for (auto &port : m_subctrl_port)
		SATURN_CONTROL_PORT(config, port, saturn_joys, "joypad");
}


uint8_t saturn_segatap_device::read_ctrl(uint8_t offset)
{
	return m_subctrl_port[offset < 8 ? (offset >> 1) : 0]->read_ctrl(offset & 1);
}

uint8_t saturn_segatap_device::read_id(int idx)
{
	return m_subctrl_port[idx < 4 ? idx : 0]->read_id(0);
}
