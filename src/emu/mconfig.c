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
	  m_options(options)
{
	// construct the config
	(*gamedrv.machine_config)(*this, NULL);

	// intialize slot devices - make sure that any required devices have been allocated
	device_slot_interface *slot = NULL;
    for (bool gotone = m_devicelist.first(slot); gotone; gotone = slot->next(slot))
	{
		const slot_interface *intf = slot->get_slot_interfaces();
		if (intf != NULL)
		{
			device_t &owner = slot->device();
			const char *selval = options.value(owner.tag());
			if (!options.exists(owner.tag()))
				selval = slot->get_default_card(devicelist(), options);

			if (selval != NULL && strlen(selval)!=0) {
				bool found = false;
				for (int i = 0; intf[i].name != NULL; i++) {
					if (strcmp(selval, intf[i].name) == 0) {
						device_t *new_dev = device_add(&owner, intf[i].name, intf[i].devtype, 0);
						found = true;
						if (!options.exists(owner.tag()))
							device_t::static_set_input_default(*new_dev, slot->input_ports_defaults());
					}
				}
				if (!found) 
					throw emu_fatalerror("Unknown slot option '%s' in slot '%s'", selval, owner.tag());
			}
		}
	}

	// when finished, set the game driver
	device_t *root = m_devicelist.find("root");
	if (root == NULL)
		throw emu_fatalerror("Machine configuration missing driver_device");
	driver_device::static_set_game(*root, gamedrv);

	// then notify all devices that their configuration is complete
	for (device_t *device = m_devicelist.first(); device != NULL; device = device->next())
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
	return downcast<screen_device *>(m_devicelist.first(SCREEN));
}


//-------------------------------------------------
//  device_add_subdevices - helper to add
//  devices owned by the device
//-------------------------------------------------

void machine_config::device_add_subdevices(device_t *device)
{
	machine_config_constructor additions = device->machine_config_additions();
	if (additions != NULL)
		(*additions)(*this, device);
}


//-------------------------------------------------
//  device_remove_subdevices - helper to remove
//  devices owned by the device
//-------------------------------------------------

void machine_config::device_remove_subdevices(const device_t *device)
{
	if (device != NULL)
	{
		device_t *sub_device = m_devicelist.first();
		while (sub_device != NULL)
		{
			if (sub_device->owner() == device)
				device_remove_subdevices(sub_device);

			device_t *next_device = sub_device->next();

			if (sub_device->owner() == device)
				m_devicelist.remove(*sub_device);

			sub_device = next_device;
		}
	}
}


//-------------------------------------------------
//  device_add - configuration helper to add a
//  new device
//-------------------------------------------------

device_t *machine_config::device_add(device_t *owner, const char *tag, device_type type, UINT32 clock)
{
	astring tempstring;
	const char *fulltag = owner->subtag(tempstring, tag);
	device_t *device = &m_devicelist.append(fulltag, *(*type)(*this, fulltag, owner, clock));
	device_add_subdevices(device);
	return device;
}


//-------------------------------------------------
//  device_replace - configuration helper to
//  replace one device with a new device
//-------------------------------------------------

device_t *machine_config::device_replace(device_t *owner, const char *tag, device_type type, UINT32 clock)
{
	astring tempstring;
	const char *fulltag = owner->subtag(tempstring, tag);
	device_remove_subdevices(m_devicelist.find(fulltag));
	device_t *device = &m_devicelist.replace_and_remove(fulltag, *(*type)(*this, fulltag, owner, clock));
	device_add_subdevices(device);
	return device;
}


//-------------------------------------------------
//  device_remove - configuration helper to
//  remove a device
//-------------------------------------------------

device_t *machine_config::device_remove(device_t *owner, const char *tag)
{
	astring tempstring;
	const char *fulltag = owner->subtag(tempstring, tag);
	device_t *device=m_devicelist.find(fulltag);
	device_remove_subdevices(device);
	m_devicelist.remove(*device);
	return NULL;
}


//-------------------------------------------------
//  device_find - configuration helper to
//  locate a device
//-------------------------------------------------

device_t *machine_config::device_find(device_t *owner, const char *tag)
{
	astring tempstring;
	const char *fulltag = owner->subtag(tempstring, tag);
	device_t *device = m_devicelist.find(fulltag);
	if (device == NULL)
		throw emu_fatalerror("Unable to find device: tag=%s\n", fulltag);
	return device;
}
