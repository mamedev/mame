/***************************************************************************

    mconfig.c

    Machine configuration macros and functions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
	  m_perfect_cpu_quantum(NULL),
	  m_watchdog_vblank_count(0),
	  m_watchdog_time(attotime::zero),
	  m_nvram_handler(NULL),
	  m_memcard_handler(NULL),
	  m_video_attributes(0),
	  m_gfxdecodeinfo(NULL),
	  m_total_colors(0),
	  m_default_layout(NULL),
	  m_gamedrv(gamedrv),
	  m_options(options),
	  m_root_device(NULL)
{
	// construct the config
	(*gamedrv.machine_config)(*this, NULL);

	bool is_selected_driver = strcmp(gamedrv.name,options.system_name())==0;
	// intialize slot devices - make sure that any required devices have been allocated
	slot_interface_iterator slotiter(root_device());
    for (device_slot_interface *slot = slotiter.first(); slot != NULL; slot = slotiter.next())
	{
		const slot_interface *intf = slot->get_slot_interfaces();
		if (intf != NULL)
		{
			device_t &owner = slot->device();
			const char *selval = options.value(owner.tag()+1);
			bool isdefault = (options.priority(owner.tag()+1)==OPTION_PRIORITY_DEFAULT);
			if (!is_selected_driver || !options.exists(owner.tag()+1))
				selval = slot->get_default_card();

			if (selval != NULL && strlen(selval) != 0)
			{
				bool found = false;
				for (int i = 0; intf[i].name != NULL; i++)
				{
					if (strcmp(selval, intf[i].name) == 0)
					{
						if ((!intf[i].internal) || (isdefault && intf[i].internal))
						{
							const char *def = slot->get_default_card();
							bool is_default = (def != NULL && strcmp(def, selval) == 0);
							device_t *new_dev = device_add(&owner, intf[i].name, intf[i].devtype, is_default ? slot->default_clock() : 0);
							found = true;
							if (is_default) {
								device_t::static_set_input_default(*new_dev, slot->input_ports_defaults());								
								if (slot->default_config()) {
									device_t::static_set_static_config(*new_dev, slot->default_config());
								}
							}
						}
					}
				}
				if (!found)
					throw emu_fatalerror("Unknown slot option '%s' in slot '%s'", selval, owner.tag()+1);
			}
		}
	}

	// when finished, set the game driver
	driver_device::static_set_game(*m_root_device, gamedrv);

	// then notify all devices that their configuration is complete
	device_iterator iter(root_device());
	for (device_t *device = iter.first(); device != NULL; device = iter.next())
		if (!device->configured())
			device->config_complete();
}


//-------------------------------------------------
//  ~machine_config - destructor
//-------------------------------------------------

machine_config::~machine_config()
{
	global_free(m_root_device);
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
		owner = m_root_device;
	}

	// go down the path until we're done with it
	while (strchr(tag, ':'))
	{
		const char *next = strchr(tag, ':');
		assert(next != tag);
		astring part(tag, next-tag);
		device_t *curdevice;
		for (curdevice = owner->m_subdevice_list.first(); curdevice != NULL; curdevice = curdevice->next())
			if (part == curdevice->m_basetag)
				break;
		if (!curdevice)
			throw emu_fatalerror("Could not find %s when looking up path for device %s\n",
								 part.cstr(), orig_tag);
		owner = curdevice;
		tag = next+1;
	}
	assert(tag[0]);

	// if there's an owner, let the owner do the work
	if (owner != NULL)
		return owner->add_subdevice(type, tag, clock);

	// otherwise, allocate the device directly
	assert(m_root_device == NULL);
	m_root_device = (*type)(*this, tag, owner, clock);

	// apply any machine configuration owned by the device now
	machine_config_constructor additions = m_root_device->machine_config_additions();
	if (additions != NULL)
		(*additions)(*this, m_root_device);
	return m_root_device;
}


//-------------------------------------------------
//  device_replace - configuration helper to
//  replace one device with a new device
//-------------------------------------------------

device_t *machine_config::device_replace(device_t *owner, const char *tag, device_type type, UINT32 clock)
{
	// find the original device by this name (must exist)
	assert(owner != NULL);
	device_t *device = owner->subdevice(tag);
	if (device == NULL)
	{
		mame_printf_warning("Warning: attempting to replace non-existent device '%s'\n", tag);
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
	assert(owner != NULL);
	device_t *device = owner->subdevice(tag);
	if (device == NULL)
	{
		mame_printf_warning("Warning: attempting to remove non-existent device '%s'\n", tag);
		return NULL;
	}

	// let the device's owner do the work
	device->owner()->remove_subdevice(*device);
	return NULL;
}


//-------------------------------------------------
//  device_find - configuration helper to
//  locate a device
//-------------------------------------------------

device_t *machine_config::device_find(device_t *owner, const char *tag)
{
	// find the original device by this name (must exist)
	assert(owner != NULL);
	device_t *device = owner->subdevice(tag);
	assert(device != NULL);
	if (device == NULL)
		throw emu_fatalerror("Unable to find device '%s'\n", tag);

	// return the device
	return device;
}
