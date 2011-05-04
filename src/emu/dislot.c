/***************************************************************************

        Slot device

***************************************************************************/

#include "emu.h"


device_slot_interface::device_slot_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device)
{
}

device_slot_interface::~device_slot_interface()
{
}


void device_slot_interface::static_set_slot_info(device_t &device, const slot_interface *slots_info, const char *default_card)
{
	device_slot_interface *slot;
	if (!device.interface(slot))
		throw emu_fatalerror("set_default_slot_card called on device '%s' with no slot interface", device.tag());
	
	slot->m_slot_interfaces = slots_info;
	slot->m_default_card = default_card;
}

device_slot_card_interface::device_slot_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device)
{
}

device_slot_card_interface::~device_slot_card_interface()
{
}

