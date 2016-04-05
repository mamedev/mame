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
		std::string selval;
		bool isdefault = (options.priority(owner.tag()+1)==OPTION_PRIORITY_DEFAULT);
		if (is_selected_driver && options.exists(owner.tag()+1))
			selval = options.main_value(owner.tag()+1);
		else if (slot->default_option() != nullptr)
			selval.assign(slot->default_option());

		if (!selval.empty())
		{
			const device_slot_option *option = slot->option(selval.c_str());

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
				throw emu_fatalerror("Unknown slot option '%s' in slot '%s'", selval.c_str(), owner.tag()+1);
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
		owner = owner->subdevices().find(part);
		if (owner == nullptr)
			throw emu_fatalerror("Could not find %s when looking up path for device %s\n",
									part.c_str(), orig_tag);
		tag = next+1;
	}
	assert(tag[0] != '\0');

	if (owner != nullptr)
	{
		// allocate the new device
		device_t *device = (*type)(*this, tag, owner, clock);

		// append it to the owner's list
		return &config_new_device(owner->subdevices().m_list.append(*device));
	}

	// allocate the root device directly
	assert(m_root_device == nullptr);
	m_root_device.reset((*type)(*this, tag, nullptr, clock));
	return &config_new_device(*m_root_device);
}


//-------------------------------------------------
//  device_replace - configuration helper to
//  replace one device with a new device
//-------------------------------------------------

device_t *machine_config::device_replace(device_t *owner, const char *tag, device_type type, UINT32 clock)
{
	// find the original device by relative tag (must exist)
	assert(owner != nullptr);
	device_t *old_device = owner->subdevice(tag);
	if (old_device == nullptr)
	{
		osd_printf_warning("Warning: attempting to replace non-existent device '%s'\n", tag);
		return device_add(owner, tag, type, clock);
	}

	// make sure we have the old device's actual owner
	owner = old_device->owner();
	assert(owner != nullptr);

	// remove references to the old device
	remove_references(*old_device);

	// allocate the new device
	device_t *new_device = (*type)(*this, tag, owner, clock);

	// substitute it for the old one in the owner's list
	return &config_new_device(owner->subdevices().m_list.replace_and_remove(*new_device, *old_device));
}


//-------------------------------------------------
//  device_remove - configuration helper to
//  remove a device
//-------------------------------------------------

device_t *machine_config::device_remove(device_t *owner, const char *tag)
{
	// find the original device by relative tag (must exist)
	assert(owner != nullptr);
	device_t *device = owner->subdevice(tag);
	if (device == nullptr)
	{
		osd_printf_warning("Warning: attempting to remove non-existent device '%s'\n", tag);
		return nullptr;
	}

	// make sure we have the old device's actual owner
	owner = device->owner();
	assert(owner != nullptr);

	// remove references to the old device
	remove_references(*device);

	// let the device's owner do the work
	owner->subdevices().m_list.remove(*device);
	return nullptr;
}


//-------------------------------------------------
//  device_find - configuration helper to
//  locate a device
//-------------------------------------------------

device_t *machine_config::device_find(device_t *owner, const char *tag)
{
	// find the original device by relative tag (must exist)
	assert(owner != nullptr);
	device_t *device = owner->subdevice(tag);
	assert(device != nullptr);
	if (device == nullptr)
		throw emu_fatalerror("Unable to find device '%s'\n", tag);

	// return the device
	return device;
}


//-------------------------------------------------
//  remove_references - globally remove references
//  to a device about to be removed from the tree
//-------------------------------------------------

void machine_config::remove_references(ATTR_UNUSED device_t &device)
{
	// iterate over all devices and remove any references
	device_iterator iter(root_device());
	for (device_t *scan = iter.first(); scan != nullptr; scan = iter.next())
		scan->subdevices().m_tagmap.clear(); //remove(&device);
}


//-------------------------------------------------
//  config_new_device - helper for recursive
//  configuration of newly added devices
//-------------------------------------------------

device_t &machine_config::config_new_device(device_t &device)
{
	// apply any machine configuration owned by the device now
	machine_config_constructor additions = device.machine_config_additions();
	if (additions != nullptr)
		(*additions)(*this, &device, nullptr);

	// return the new device
	return device;
}
