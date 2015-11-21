// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Slot device

***************************************************************************/

#include "emu.h"
#include "emuopts.h"

device_slot_interface::device_slot_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "slot"),
	m_default_option(NULL),
	m_fixed(false)
{
}

device_slot_interface::~device_slot_interface()
{
}

device_slot_option::device_slot_option(const char *name, const device_type &devtype): 
	m_next(NULL),
    m_name(name),
	m_devtype(devtype),
	m_selectable(true),
	m_default_bios(NULL),
	m_machine_config(NULL),
	m_input_device_defaults(NULL),
	m_clock(0)
{
}

void device_slot_interface::static_option_reset(device_t &device)
{
	device_slot_interface &intf = dynamic_cast<device_slot_interface &>(device);

	intf.m_options.reset();
}

void device_slot_interface::static_option_add(device_t &device, const char *name, const device_type &devtype)
{
	device_slot_interface &intf = dynamic_cast<device_slot_interface &>(device);
	device_slot_option *option = intf.option(name);

	if (option != NULL)
		throw emu_fatalerror("slot '%s' duplicate option '%s\n", device.tag(), name);

	intf.m_options.append(name, *global_alloc(device_slot_option(name, devtype)));
}

device_slot_option *device_slot_interface::static_option(device_t &device, const char *name)
{
	device_slot_interface &intf = dynamic_cast<device_slot_interface &>(device);
	device_slot_option *option = intf.option(name);

	if (option == NULL)
		throw emu_fatalerror("slot '%s' has no option '%s\n", device.tag(), name);

	return option;
}

device_t* device_slot_interface::get_card_device()
{
	const char *subtag;
	device_t *dev = NULL;
	std::string temp;
	if (!device().mconfig().options().exists(device().tag()+1)) {
		subtag = m_default_option;
	} else {
		subtag = device().mconfig().options().main_value(temp,device().tag()+1);
	}
	if (subtag && *subtag != 0) {
		device_slot_card_interface *intf = NULL;
		dev = device().subdevice(subtag);
		if (dev!=NULL && !dev->interface(intf))
			throw emu_fatalerror("get_card_device called for device '%s' with no slot card interface", dev->tag());
	}
	return dev;
}


device_slot_card_interface::device_slot_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "slot")
{
}

device_slot_card_interface::~device_slot_card_interface()
{
}
