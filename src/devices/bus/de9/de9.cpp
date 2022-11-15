// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Generic DE-9 port emulation

**********************************************************************/

#include "emu.h"
#include "de9.h"


DEFINE_DEVICE_TYPE(DE9_PORT, de9_port_device, "de9_port", "DE-9 port")


device_de9_port_interface::device_de9_port_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "de9")
{
	m_port = dynamic_cast<de9_port_device *>(device.owner());
}

de9_port_device::de9_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DE9_PORT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_device(nullptr)
{
}

void de9_port_device::device_start()
{
	m_device = dynamic_cast<device_de9_port_interface *>(get_card_device());
}
