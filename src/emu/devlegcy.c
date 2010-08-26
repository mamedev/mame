/***************************************************************************

    devlegcy.c

    Legacy device helpers.

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
#include "devlegcy.h"


//**************************************************************************
//  LEGACY DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  legacy_device_config_base - constructor
//-------------------------------------------------

legacy_device_config_base::legacy_device_config_base(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock, device_get_config_func get_config)
	: device_config(mconfig, type, "Legacy Device", tag, owner, clock),
	  m_get_config_func(get_config),
	  m_inline_config(NULL)
{
	// allocate a buffer for the inline configuration
	UINT32 configlen = (UINT32)get_legacy_config_int(DEVINFO_INT_INLINE_CONFIG_BYTES);
	if (configlen != 0)
		m_inline_config = global_alloc_array_clear(UINT8, configlen);

	// set the proper name
	m_name = get_legacy_config_string(DEVINFO_STR_NAME);
}


//-------------------------------------------------
//  ~legacy_device_config_base - destructor
//-------------------------------------------------

legacy_device_config_base::~legacy_device_config_base()
{
	global_free(m_inline_config);
}


//-------------------------------------------------
//  get_legacy_config_int - return a legacy
//  configuration parameter as an integer
//-------------------------------------------------

INT64 legacy_device_config_base::get_legacy_config_int(UINT32 state) const
{
	deviceinfo info = { 0 };
	(*m_get_config_func)(this, state, &info);
	return info.i;
}


//-------------------------------------------------
//  get_legacy_config_ptr - return a legacy
//  configuration parameter as a pointer
//-------------------------------------------------

void *legacy_device_config_base::get_legacy_config_ptr(UINT32 state) const
{
	deviceinfo info = { 0 };
	(*m_get_config_func)(this, state, &info);
	return info.p;
}


//-------------------------------------------------
//  get_legacy_config_fct - return a legacy
//  configuration parameter as a function pointer
//-------------------------------------------------

genf *legacy_device_config_base::get_legacy_config_fct(UINT32 state) const
{
	deviceinfo info = { 0 };
	(*m_get_config_func)(this, state, &info);
	return info.f;
}


//-------------------------------------------------
//  get_legacy_config_string - return a legacy
//  configuration parameter as a string pointer
//-------------------------------------------------

const char *legacy_device_config_base::get_legacy_config_string(UINT32 state) const
{
	deviceinfo info;
	info.s = get_temp_string_buffer();
	(*m_get_config_func)(this, state, &info);
	return info.s;
}


//-------------------------------------------------
//  static_set_inline32 - configuration helper to
//  set a 32-bit value in the inline configuration
//-------------------------------------------------

void legacy_device_config_base::static_set_inline32(device_config *device, UINT32 offset, UINT32 size, UINT32 value)
{
	legacy_device_config_base *legacy = downcast<legacy_device_config_base *>(device);
	void *dest = reinterpret_cast<UINT8 *>(legacy->m_inline_config) + offset;
	if (size == 1)
		*reinterpret_cast<UINT8 *>(dest) = value;
	else if (size == 2)
		*reinterpret_cast<UINT16 *>(dest) = value;
	else if (size == 4)
		*reinterpret_cast<UINT32 *>(dest) = value;
	else
		throw emu_fatalerror("Unexpected size %d in legacy_device_config_base::static_set_inline32", size);
}


//-------------------------------------------------
//  static_set_inline64 - configuration helper to
//  set a 64-bit value in the inline configuration
//-------------------------------------------------

void legacy_device_config_base::static_set_inline64(device_config *device, UINT32 offset, UINT32 size, UINT64 value)
{
	legacy_device_config_base *legacy = downcast<legacy_device_config_base *>(device);
	void *dest = reinterpret_cast<UINT8 *>(legacy->m_inline_config) + offset;
	if (size == 1)
		*reinterpret_cast<UINT8 *>(dest) = value;
	else if (size == 2)
		*reinterpret_cast<UINT16 *>(dest) = value;
	else if (size == 4)
		*reinterpret_cast<UINT32 *>(dest) = value;
	else if (size == 8)
		*reinterpret_cast<UINT64 *>(dest) = value;
	else
		throw emu_fatalerror("Unexpected size %d in legacy_device_config_base::static_set_inline64", size);
}


//-------------------------------------------------
//  static_set_inline_float - configuration helper 
//  to set a floating-point value in the inline
//  configuration
//-------------------------------------------------

void legacy_device_config_base::static_set_inline_float(device_config *device, UINT32 offset, UINT32 size, float value)
{
	legacy_device_config_base *legacy = downcast<legacy_device_config_base *>(device);
	void *dest = reinterpret_cast<UINT8 *>(legacy->m_inline_config) + offset;
	if (size == 4)
		*reinterpret_cast<float *>(dest) = value;
	else
		throw emu_fatalerror("Unexpected size %d in legacy_device_config_base::static_set_inline_float", size);
}


//-------------------------------------------------
//  device_validity_check - perform validity
//  checks on a device configuration
//-------------------------------------------------

bool legacy_device_config_base::device_validity_check(const game_driver &driver) const
{
	device_validity_check_func validity_func = reinterpret_cast<device_validity_check_func>(get_legacy_config_fct(DEVINFO_FCT_VALIDITY_CHECK));
	if (validity_func != NULL)
		return (*validity_func)(&driver, this);
	return false;
}



//**************************************************************************
//  LIVE LEGACY DEVICE
//**************************************************************************

//-------------------------------------------------
//  legacy_device_base - constructor
//-------------------------------------------------

legacy_device_base::legacy_device_base(running_machine &_machine, const device_config &config)
	: device_t(_machine, config),
	  m_config(downcast<const legacy_device_config_base &>(config)),
	  m_token(NULL)
{
	int tokenbytes = m_config.get_legacy_config_int(DEVINFO_INT_TOKEN_BYTES);
	if (tokenbytes != 0)
		m_token = auto_alloc_array_clear(machine, UINT8, tokenbytes);
}


//-------------------------------------------------
//  ~legacy_device_base - destructor
//-------------------------------------------------

legacy_device_base::~legacy_device_base()
{
	if (m_started)
	{
		device_stop_func stop_func = reinterpret_cast<device_stop_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_STOP));
		if (stop_func != NULL)
			(*stop_func)(this);
	}
}


//-------------------------------------------------
//  device_start - called to start up a device
//-------------------------------------------------

void legacy_device_base::device_start()
{
	device_start_func start_func = reinterpret_cast<device_start_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_START));
	assert(start_func != NULL);
	(*start_func)(this);
}


//-------------------------------------------------
//  device_reset - called to reset a device
//-------------------------------------------------

void legacy_device_base::device_reset()
{
	device_reset_func reset_func = reinterpret_cast<device_reset_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_RESET));
	if (reset_func != NULL)
		(*reset_func)(this);
}



//**************************************************************************
//  LEGACY SOUND DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  legacy_sound_device_config_base - constructor
//-------------------------------------------------

legacy_sound_device_config_base::legacy_sound_device_config_base(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock, device_get_config_func get_config)
	: legacy_device_config_base(mconfig, type, tag, owner, clock, get_config),
	  device_config_sound_interface(mconfig, *this)
{
}



//**************************************************************************
//  LIVE LEGACY SOUND DEVICE
//**************************************************************************

//-------------------------------------------------
//  legacy_sound_device_base - constructor
//-------------------------------------------------

legacy_sound_device_base::legacy_sound_device_base(running_machine &machine, const device_config &config)
	: legacy_device_base(machine, config),
	  device_sound_interface(machine, config, *this)
{
}



//**************************************************************************
//  LEGACY MEMORY DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  legacy_memory_device_config_base - constructor
//-------------------------------------------------

legacy_memory_device_config_base::legacy_memory_device_config_base(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock, device_get_config_func get_config)
	: legacy_device_config_base(mconfig, type, tag, owner, clock, get_config),
	  device_config_memory_interface(mconfig, *this)
{
	memset(&m_space_config, 0, sizeof(m_space_config));
}


//-------------------------------------------------
//  device_config_complete - update configuration
//  based on completed device setup
//-------------------------------------------------

void legacy_memory_device_config_base::device_config_complete()
{
	m_space_config.m_name = "memory";
	m_space_config.m_endianness = static_cast<endianness_t>(get_legacy_config_int(DEVINFO_INT_ENDIANNESS));
	m_space_config.m_databus_width = get_legacy_config_int(DEVINFO_INT_DATABUS_WIDTH);
	m_space_config.m_addrbus_width = get_legacy_config_int(DEVINFO_INT_ADDRBUS_WIDTH);
	m_space_config.m_addrbus_shift = get_legacy_config_int(DEVINFO_INT_ADDRBUS_SHIFT);
	m_space_config.m_logaddr_width = m_space_config.m_addrbus_width;
	m_space_config.m_page_shift = 0;
	m_space_config.m_internal_map = reinterpret_cast<address_map_constructor>(get_legacy_config_fct(DEVINFO_PTR_INTERNAL_MEMORY_MAP));
	m_space_config.m_default_map = reinterpret_cast<address_map_constructor>(get_legacy_config_fct(DEVINFO_PTR_DEFAULT_MEMORY_MAP));
}



//**************************************************************************
//  LIVE LEGACY MEMORY DEVICE
//**************************************************************************

//-------------------------------------------------
//  legacy_memory_device_base - constructor
//-------------------------------------------------

legacy_memory_device_base::legacy_memory_device_base(running_machine &machine, const device_config &config)
	: legacy_device_base(machine, config),
	  device_memory_interface(machine, config, *this)
{
}



//**************************************************************************
//  LEGACY NVRAM DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  legacy_nvram_device_config_base - constructor
//-------------------------------------------------

legacy_nvram_device_config_base::legacy_nvram_device_config_base(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock, device_get_config_func get_config)
	: legacy_device_config_base(mconfig, type, tag, owner, clock, get_config),
	  device_config_nvram_interface(mconfig, *this)
{
}



//**************************************************************************
//  LIVE LEGACY NVRAM DEVICE
//**************************************************************************

//-------------------------------------------------
//  legacy_nvram_device_base - constructor
//-------------------------------------------------

legacy_nvram_device_base::legacy_nvram_device_base(running_machine &machine, const device_config &config)
	: legacy_device_base(machine, config),
	  device_nvram_interface(machine, config, *this)
{
}


//-------------------------------------------------
//  nvram_default - generate the default NVRAM
//-------------------------------------------------

void legacy_nvram_device_base::nvram_default()
{
	device_nvram_func nvram_func = reinterpret_cast<device_nvram_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_NVRAM));
	(*nvram_func)(this, NULL, FALSE);
}


//-------------------------------------------------
//  nvram_read - read NVRAM from the given file
//-------------------------------------------------

void legacy_nvram_device_base::nvram_read(mame_file &file)
{
	device_nvram_func nvram_func = reinterpret_cast<device_nvram_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_NVRAM));
	(*nvram_func)(this, &file, FALSE);
}


//-------------------------------------------------
//  nvram_write - write NVRAM to the given file
//-------------------------------------------------

void legacy_nvram_device_base::nvram_write(mame_file &file)
{
	device_nvram_func nvram_func = reinterpret_cast<device_nvram_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_NVRAM));
	(*nvram_func)(this, &file, TRUE);
}
