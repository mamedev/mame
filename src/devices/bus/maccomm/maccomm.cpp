// license:BSD-3-Clause
// copyright-holders:Tim Douglas
/***************************************************************************

    Apple Communication Slot

***************************************************************************/

#include "emu.h"
#include "maccomm.h"

DEFINE_DEVICE_TYPE(MAC_COMM_SLOT, mac_comm_slot_device, "maccomm_slot", "Apple Communication Slot")


mac_comm_slot_device::mac_comm_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MAC_COMM_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_mac_comm_card_interface>(mconfig, *this)
	, m_space(*this, finder_base::DUMMY_TAG, -1)
	, m_irq_cb(*this)
{
}

void mac_comm_slot_device::device_start()
{
	device_mac_comm_card_interface *const card = get_card_device();
	if (card)
		card->install(*m_space, 0x50f0'8000);
}


device_mac_comm_card_interface::device_mac_comm_card_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "maccomm")
	, m_slot(dynamic_cast<mac_comm_slot_device *>(device.owner()))
{
}

void device_mac_comm_card_interface::interface_validity_check(validity_checker &valid) const
{
	device_t *const owner(device().owner());
	if (owner && dynamic_cast<device_slot_interface *>(owner) && !dynamic_cast<mac_comm_slot_device *>(owner))
		osd_printf_error("Device %s (%s) is not an Apple Communication Slot\n", owner->tag(), owner->name());
}
