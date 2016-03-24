// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Slot device

***************************************************************************/

#include "emu.h"
#include "emuopts.h"

device_slot_interface::device_slot_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "slot"),
	m_default_option(nullptr),
	m_fixed(false)
{
}

device_slot_interface::~device_slot_interface()
{
}

device_slot_option::device_slot_option(const char *name, const device_type &devtype):
	m_next(nullptr),
	m_name(name),
	m_devtype(devtype),
	m_selectable(true),
	m_default_bios(nullptr),
	m_machine_config(nullptr),
	m_input_device_defaults(nullptr),
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

	if (option != nullptr)
		throw emu_fatalerror("slot '%s' duplicate option '%s\n", device.tag(), name);

	intf.m_options.append(name, *global_alloc(device_slot_option(name, devtype)));
}

device_slot_option *device_slot_interface::static_option(device_t &device, const char *name)
{
	device_slot_interface &intf = dynamic_cast<device_slot_interface &>(device);
	device_slot_option *option = intf.option(name);

	if (option == nullptr)
		throw emu_fatalerror("slot '%s' has no option '%s\n", device.tag(), name);

	return option;
}

device_t* device_slot_interface::get_card_device()
{
	std::string subtag;
	device_t *dev = nullptr;
	if (device().mconfig().options().exists(device().tag()+1))
		subtag = device().mconfig().options().main_value(device().tag()+1);
	else if (m_default_option != nullptr)
		subtag.assign(m_default_option);
	if (!subtag.empty()) {
		device_slot_card_interface *intf = nullptr;
		dev = device().subdevice(subtag.c_str());
		if (dev!=nullptr && !dev->interface(intf))
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
