/***************************************************************************

    nvram.c

    Generic non-volatile RAM.

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
#include "machine/nvram.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NVRAM = nvram_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  nvram_device_config - constructor
//-------------------------------------------------

nvram_device_config::nvram_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "NVRAM", tag, owner, clock),
	  device_config_nvram_interface(mconfig, *this),
	  m_default_value(DEFAULT_ALL_1)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *nvram_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(nvram_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *nvram_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, nvram_device(machine, *this));
}


//-------------------------------------------------
//  static_set_interface - configuration helper
//  to set the interface
//-------------------------------------------------

void nvram_device_config::static_set_default_value(device_config *device, default_value value)
{
	nvram_device_config *nvram = downcast<nvram_device_config *>(device);
	nvram->m_default_value = value;
}


//-------------------------------------------------
//  static_set_custom_handler - configuration
//  helper to set a custom callback
//-------------------------------------------------

void nvram_device_config::static_set_custom_handler(device_config *device, nvram_init_proto_delegate handler)
{
	nvram_device_config *nvram = downcast<nvram_device_config *>(device);
	nvram->m_default_value = DEFAULT_CUSTOM;
	nvram->m_custom_handler = handler;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nvram_device - constructor
//-------------------------------------------------

nvram_device::nvram_device(running_machine &_machine, const nvram_device_config &config)
	: device_t(_machine, config),
	  device_nvram_interface(_machine, config, *this),
	  m_config(config),
	  m_base(NULL),
	  m_length(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nvram_device::device_start()
{
	// bind our handler
	if (!m_config.m_custom_handler.isnull())
		m_custom_handler = nvram_init_delegate(m_config.m_custom_handler, *m_owner);
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void nvram_device::nvram_default()
{
	// make sure we have a valid base pointer
	determine_final_base();

	// region always wins
	if (m_region != NULL)
	{
		memcpy(m_base, *m_region, m_length);
		return;
	}

	// default values for other cases
	switch (m_config.m_default_value)
	{
		// all-0's
		case nvram_device_config::DEFAULT_ALL_0:
			memset(m_base, 0, m_length);
			break;

		// all 1's
		default:
		case nvram_device_config::DEFAULT_ALL_1:
			memset(m_base, 0xff, m_length);
			break;

		// random values
		case nvram_device_config::DEFAULT_RANDOM:
		{
			UINT8 *nvram = reinterpret_cast<UINT8 *>(m_base);
			for (int index = 0; index < m_length; index++)
				nvram[index] = mame_rand(&m_machine);
			break;
		}

		// custom handler
		case nvram_device_config::DEFAULT_CUSTOM:
			m_custom_handler(*this, m_base, m_length);
			break;
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void nvram_device::nvram_read(mame_file &file)
{
	// make sure we have a valid base pointer
	determine_final_base();

	mame_fread(&file, m_base, m_length);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void nvram_device::nvram_write(mame_file &file)
{
	mame_fwrite(&file, m_base, m_length);
}


//-------------------------------------------------
//  determine_final_base - get the final base
//  pointer by looking up the memory share, unless
//  a pointer was provided to us
//-------------------------------------------------

void nvram_device::determine_final_base()
{
	// find our shared pointer with the target RAM
	if (m_base == NULL)
	{
		m_base = memory_get_shared(m_machine, tag(), m_length);
		if (m_base == NULL)
			throw emu_fatalerror("NVRAM device '%s' has no corresponding AM_SHARE region", tag());
	}

	// if we are region-backed for the default, find it now and make sure it's the right size
	if (m_region != NULL && m_region->bytes() != m_length)
		throw emu_fatalerror("NVRAM device '%s' has a default region, but it should be 0x%X bytes", tag(), m_length);
}
