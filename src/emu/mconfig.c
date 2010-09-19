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
#include <ctype.h>


//**************************************************************************
//  MACHINE CONFIGURATIONS
//**************************************************************************

//-------------------------------------------------
//  machine_config - constructor
//-------------------------------------------------

machine_config::machine_config(const game_driver &gamedrv)
	: m_minimum_quantum(attotime_zero),
	  m_perfect_cpu_quantum(NULL),
	  m_watchdog_vblank_count(0),
	  m_watchdog_time(attotime_zero),
	  m_nvram_handler(NULL),
	  m_memcard_handler(NULL),
	  m_video_attributes(0),
	  m_gfxdecodeinfo(NULL),
	  m_total_colors(0),
	  m_default_layout(NULL),
	  m_gamedrv(gamedrv),
	  m_parse_level(0)
{
	// construct the config
	(*gamedrv.machine_config)(*this, NULL);

	// when finished, set the game driver
	device_config *config = m_devicelist.find("root");
	if (config == NULL)
		throw emu_fatalerror("Machine configuration missing driver_device");
	driver_device_config_base::static_set_game(config, &gamedrv);

	// process any device-specific machine configurations
	for (device_config *devconfig = m_devicelist.first(); devconfig != NULL; devconfig = devconfig->next())
		if (!devconfig->m_config_complete)
		{
			machine_config_constructor additions = devconfig->machine_config_additions();
			if (additions != NULL)
				(*additions)(*this, devconfig);
		}

	// then notify all devices that their configuration is complete
	for (device_config *devconfig = m_devicelist.first(); devconfig != NULL; devconfig = devconfig->next())
		if (!devconfig->m_config_complete)
		{
			devconfig->config_complete();
			devconfig->m_config_complete = true;
		}
}


//-------------------------------------------------
//  ~machine_config - destructor
//-------------------------------------------------

machine_config::~machine_config()
{
}


//-------------------------------------------------
//  device_add - configuration helper to add a
//  new device
//-------------------------------------------------

device_config *machine_config::device_add(device_config *owner, const char *tag, device_type type, UINT32 clock)
{
	astring tempstring;
	const char *fulltag = owner->subtag(tempstring, tag);
	return m_devicelist.append(fulltag, (*type)(*this, fulltag, owner, clock));
}


//-------------------------------------------------
//  device_replace - configuration helper to
//  replace one device with a new device
//-------------------------------------------------

device_config *machine_config::device_replace(device_config *owner, const char *tag, device_type type, UINT32 clock)
{
	astring tempstring;
	const char *fulltag = owner->subtag(tempstring, tag);
	return m_devicelist.replace(fulltag, (*type)(*this, fulltag, owner, clock));
}


//-------------------------------------------------
//  device_remove - configuration helper to
//  remove a device
//-------------------------------------------------

device_config *machine_config::device_remove(device_config *owner, const char *tag)
{
	astring tempstring;
	const char *fulltag = owner->subtag(tempstring, tag);
	m_devicelist.remove(fulltag);
	return NULL;
}


//-------------------------------------------------
//  device_find - configuration helper to
//  locate a device
//-------------------------------------------------

device_config *machine_config::device_find(device_config *owner, const char *tag)
{
	astring tempstring;
	const char *fulltag = owner->subtag(tempstring, tag);
	device_config *device = m_devicelist.find(fulltag);
	if (device == NULL)
		throw emu_fatalerror("Unable to find device: tag=%s\n", fulltag);
	return device;
}
