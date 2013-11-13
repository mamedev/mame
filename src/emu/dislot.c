// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Slot device

***************************************************************************/

#include "emu.h"
#include "emuopts.h"

device_slot_interface::device_slot_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device),
	m_slot_interfaces(NULL),
	m_default_card(NULL),
	m_fixed(false)
{
}

device_slot_interface::~device_slot_interface()
{
}

void device_slot_interface::static_set_slot_info(device_t &device, const slot_interface *slots_info, const char *default_card, bool fixed)
{
	device_slot_interface *slot;
	if (!device.interface(slot))
		throw emu_fatalerror("set_default_slot_card called on device '%s' with no slot interface", device.tag());

	slot->m_slot_interfaces = slots_info;
	slot->m_default_card = default_card;
	slot->m_fixed = fixed;
}

device_card_options *device_slot_interface::static_alloc_card_options(device_t &device, const char *card)
{
	device_slot_interface &intf = dynamic_cast<device_slot_interface &>(device);

	device_card_options *options = intf.m_card_options.find(card);
	if (options == NULL)
	{
		options = pool_alloc(intf.m_card_options.pool(), device_card_options());
		intf.m_card_options.append(card, *options);
	}

	return options;
}

void device_slot_interface::static_set_card_default_bios(device_t &device, const char *card, const char *default_bios)
{
	static_alloc_card_options(device, card)->m_default_bios = default_bios;
}

void device_slot_interface::static_set_card_machine_config(device_t &device, const char *card, const machine_config_constructor machine_config)
{
	static_alloc_card_options(device, card)->m_machine_config = machine_config;
}

void device_slot_interface::static_set_card_device_input_defaults(device_t &device, const char *card, const input_device_default *input_device_defaults)
{
	static_alloc_card_options(device, card)->m_input_device_defaults = input_device_defaults;
}

void device_slot_interface::static_set_card_config(device_t &device, const char *card, const void *config)
{
	static_alloc_card_options(device, card)->m_config = config;
}

void device_slot_interface::static_set_card_clock(device_t &device, const char *card, UINT32 clock)
{
	static_alloc_card_options(device, card)->m_clock = clock;
}

const char *device_slot_interface::card_default_bios(const char *card) const
{
	device_card_options *options = m_card_options.find(card);
	if (options != NULL)
		return options->m_default_bios;
	return NULL;
}

const machine_config_constructor device_slot_interface::card_machine_config(const char *card) const
{
	device_card_options *options = m_card_options.find(card);
	if (options != NULL)
		return options->m_machine_config;
	return NULL;
}

const input_device_default *device_slot_interface::card_input_device_defaults(const char *card) const
{
	device_card_options *options = m_card_options.find(card);
	if (options != NULL)
		return options->m_input_device_defaults;
	return NULL;
}

const void *device_slot_interface::card_config(const char *card) const
{
	device_card_options *options = m_card_options.find(card);
	if (options != NULL)
		return options->m_config;
	return NULL;
}

const UINT32 device_slot_interface::card_clock(const char *card) const
{
	device_card_options *options = m_card_options.find(card);
	if (options != NULL)
		return options->m_clock;
	return 0;
}

device_t* device_slot_interface::get_card_device()
{
	const char *subtag;
	device_t *dev = NULL;
	astring temp;
	if (!device().mconfig().options().exists(device().tag()+1)) {
		subtag = m_default_card;
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

const bool device_slot_interface::all_internal() const
{
	for (int i = 0; m_slot_interfaces && m_slot_interfaces[i].name != NULL; i++)
		if (!m_slot_interfaces[i].internal)
			return FALSE;
	return TRUE;
}


bool device_slot_interface::is_internal_option(const char *option) const
{
	if ( !option )
	{
		return false;
	}

	for (int i = 0; m_slot_interfaces && m_slot_interfaces[i].name != NULL; i++)
	{
		if ( !strcmp(m_slot_interfaces[i].name, option) )
		{
			return m_slot_interfaces[i].internal;
		}
	}
	return false;
}


device_slot_card_interface::device_slot_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device)
{
}

device_slot_card_interface::~device_slot_card_interface()
{
}
