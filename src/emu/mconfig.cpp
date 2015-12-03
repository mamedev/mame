// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mconfig.c

    Machine configuration macros and functions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include <ctype.h>


//**************************************************************************
//  MACHINE CONFIGURATIONS
//**************************************************************************

//-------------------------------------------------
//  machine_config - constructor
//-------------------------------------------------

machine_config::machine_config(const game_driver &gamedrv, emu_options &options)
	: m_minimum_quantum(attotime::zero),
		m_watchdog_vblank_count(0),
		m_watchdog_time(attotime::zero),
		m_force_no_drc(false),
		m_default_layout(nullptr),
		m_gamedrv(gamedrv),
		m_options(options)
{
	// construct the config
	(*gamedrv.machine_config)(*this, nullptr, nullptr);

	bool is_selected_driver = core_stricmp(gamedrv.name,options.system_name())==0;
	// intialize slot devices - make sure that any required devices have been allocated
	slot_interface_iterator slotiter(root_device());
	for (device_slot_interface *slot = slotiter.first(); slot != nullptr; slot = slotiter.next())
	{
		device_t &owner = slot->device();
		std::string temp;
		const char *selval = options.main_value(temp, owner.tag()+1);
		bool isdefault = (options.priority(owner.tag()+1)==OPTION_PRIORITY_DEFAULT);
		if (!is_selected_driver || !options.exists(owner.tag()+1))
			selval = slot->default_option();

		if (selval != nullptr && *selval != 0)
		{
			const device_slot_option *option = slot->option(selval);

			if (option && (isdefault || option->selectable()))
			{
				device_t *new_dev = device_add(&owner, option->name(), option->devtype(), option->clock());

				const char *default_bios = option->default_bios();
				if (default_bios != nullptr)
					device_t::static_set_default_bios_tag(*new_dev, default_bios);

				machine_config_constructor additions = option->machine_config();
				if (additions != nullptr)
					(*additions)(const_cast<machine_config &>(*this), new_dev, new_dev);

				const input_device_default *input_device_defaults = option->input_device_defaults();
				if (input_device_defaults)
					device_t::static_set_input_default(*new_dev, input_device_defaults);
			}
			else
				throw emu_fatalerror("Unknown slot option '%s' in slot '%s'", selval, owner.tag()+1);
		}
	}

	// when finished, set the game driver
	driver_device::static_set_game(*m_root_device, gamedrv);

	// then notify all devices that their configuration is complete
	device_iterator iter(root_device());
	for (device_t *device = iter.first(); device != nullptr; device = iter.next())
		if (!device->configured())
			device->config_complete();
}


//-------------------------------------------------
//  ~machine_config - destructor
//-------------------------------------------------

machine_config::~machine_config()
{
}


//-------------------------------------------------
//  first_screen - return a pointer to the first
//  screen device
//-------------------------------------------------

screen_device *machine_config::first_screen() const
{
	screen_device_iterator iter(root_device());
	return iter.first();
}


//-------------------------------------------------
//  device_add - configuration helper to add a
//  new device
//-------------------------------------------------

device_t *machine_config::device_add(device_t *owner, const char *tag, device_type type, UINT32 clock)
{
	const char *orig_tag = tag;

	// if the device path is absolute, start from the root
	if (tag[0] == ':')
	{
		tag++;
		owner = m_root_device.get();
	}

	// go down the path until we're done with it
	while (strchr(tag, ':'))
	{
		const char *next = strchr(tag, ':');
		assert(next != tag);
		std::string part(tag, next-tag);
		device_t *curdevice;
		for (curdevice = owner->m_subdevice_list.first(); curdevice != nullptr; curdevice = curdevice->next())
			if (part.compare(curdevice->m_basetag)==0)
				break;
		if (!curdevice)
			throw emu_fatalerror("Could not find %s when looking up path for device %s\n",
									part.c_str(), orig_tag);
		owner = curdevice;
		tag = next+1;
	}
	assert(tag[0]);

	// if there's an owner, let the owner do the work
	if (owner != nullptr)
		return owner->add_subdevice(type, tag, clock);

	// otherwise, allocate the device directly
	assert(m_root_device == nullptr);
	m_root_device.reset((*type)(*this, tag, owner, clock));

	// apply any machine configuration owned by the device now
	machine_config_constructor additions = m_root_device->machine_config_additions();
	if (additions != nullptr)
		(*additions)(*this, m_root_device.get(), nullptr);
	return m_root_device.get();
}


//-------------------------------------------------
//  device_replace - configuration helper to
//  replace one device with a new device
//-------------------------------------------------

device_t *machine_config::device_replace(device_t *owner, const char *tag, device_type type, UINT32 clock)
{
	// find the original device by this name (must exist)
	assert(owner != nullptr);
	device_t *device = owner->subdevice(tag);
	if (device == nullptr)
	{
		osd_printf_warning("Warning: attempting to replace non-existent device '%s'\n", tag);
		return device_add(owner, tag, type, clock);
	}

	// let the device's owner do the work
	return device->owner()->replace_subdevice(*device, type, tag, clock);
}


//-------------------------------------------------
//  device_remove - configuration helper to
//  remove a device
//-------------------------------------------------

device_t *machine_config::device_remove(device_t *owner, const char *tag)
{
	// find the original device by this name (must exist)
	assert(owner != nullptr);
	device_t *device = owner->subdevice(tag);
	if (device == nullptr)
	{
		osd_printf_warning("Warning: attempting to remove non-existent device '%s'\n", tag);
		return nullptr;
	}

	// let the device's owner do the work
	device->owner()->remove_subdevice(*device);
	return nullptr;
}


//-------------------------------------------------
//  device_find - configuration helper to
//  locate a device
//-------------------------------------------------

device_t *machine_config::device_find(device_t *owner, const char *tag)
{
	// find the original device by this name (must exist)
	assert(owner != nullptr);
	device_t *device = owner->subdevice(tag);
	assert(device != nullptr);
	if (device == nullptr)
		throw emu_fatalerror("Unable to find device '%s'\n", tag);

	// return the device
	return device;
}
