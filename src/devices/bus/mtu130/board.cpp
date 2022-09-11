// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// MTU-130 extension board

#include "emu.h"
#include "board.h"

DEFINE_DEVICE_TYPE(MTU130_EXTENSION, mtu130_extension_device, "mtu130_extension", "MTU130 extension board slot")

mtu130_extension_device::mtu130_extension_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MTU130_EXTENSION, tag, owner, clock),
	device_single_card_slot_interface<mtu130_extension_interface>(mconfig, *this),
	m_irq_merger(*this, finder_base::DUMMY_TAG)
{
}

void mtu130_extension_device::map_io(address_space_installer &space)
{
	auto dev = get_card_device();
	if(dev)
		dev->map_io(space);
}

void mtu130_extension_device::device_start()
{
}

mtu130_extension_interface::mtu130_extension_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "extension_board")
{
}

void mtu130_extension_interface::set_irq(bool state) const
{
	downcast<mtu130_extension_device *>(device().owner())->set_irq(state);
}
