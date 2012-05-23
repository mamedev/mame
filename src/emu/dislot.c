/***************************************************************************

        Slot device

***************************************************************************/

#include "emu.h"
#include "emuopts.h"

device_slot_interface::device_slot_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device)
{
	m_default_card = 0;
	m_input_defaults = 0;
	m_slot_interfaces = 0;
}

device_slot_interface::~device_slot_interface()
{
}


void device_slot_interface::static_set_slot_info(device_t &device, const slot_interface *slots_info, const char *default_card,const input_device_default *default_input, bool fixed)
{
	device_slot_interface *slot;
	if (!device.interface(slot))
		throw emu_fatalerror("set_default_slot_card called on device '%s' with no slot interface", device.tag());

	slot->m_slot_interfaces = slots_info;
	slot->m_default_card = default_card;
	slot->m_input_defaults = default_input;
	slot->m_fixed = fixed;
}

device_t* device_slot_interface::get_card_device()
{
	const char *subtag;
	device_t *dev = NULL;
	if (!device().mconfig().options().exists(device().tag()+1)) {
		subtag = m_default_card;
	} else {
		subtag = device().mconfig().options().value(device().tag()+1);
	}
	if (subtag && strlen(subtag)>0) {
		device_slot_card_interface *intf = NULL;
		dev = device().subdevice(subtag);
		if (dev!=NULL && !dev->interface(intf))
			throw emu_fatalerror("get_card_device called for device '%s' with no slot card interface", dev->tag());
	}
	return dev;
}

device_slot_card_interface::device_slot_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device)
{
}

device_slot_card_interface::~device_slot_card_interface()
{
}

