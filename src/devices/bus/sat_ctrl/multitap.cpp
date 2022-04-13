// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Saturn  multitap emulation

**********************************************************************/

#include "emu.h"
#include "multitap.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SATURN_MULTITAP, saturn_multitap_device, "saturn_multitap", "Sega Saturn Multitap")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

saturn_multitap_device::saturn_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SATURN_MULTITAP, tag, owner, clock)
	, device_saturn_control_port_interface(mconfig, *this)
	, m_subctrl_port(*this, "ctrl%u", 1U)
{
}


void saturn_multitap_device::device_start()
{
}

void saturn_multitap_device::device_add_mconfig(machine_config &config)
{
	for (auto &port : m_subctrl_port)
		SATURN_CONTROL_PORT(config, port, saturn_joys, "joypad");
}


uint8_t saturn_multitap_device::read_ctrl(uint8_t offset)
{
	return m_subctrl_port[offset < 12 ? (offset >> 1) : 0]->read_ctrl(offset & 1);
}

uint8_t saturn_multitap_device::read_id(int idx)
{
	return m_subctrl_port[idx < 6 ? idx : 0]->read_id(0);
}
