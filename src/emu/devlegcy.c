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
	: device_config(mconfig, type, tag, owner, clock),
	  m_get_config_func(get_config),
	  m_inline_config(NULL)
{
	// allocate a buffer for the inline configuration
	UINT32 configlen = (UINT32)get_legacy_config_int(DEVINFO_INT_INLINE_CONFIG_BYTES);
	if (configlen != 0)
		m_inline_config = global_alloc_array_clear(UINT8, configlen);
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
	m_space_config.m_internal_map = reinterpret_cast<const addrmap_token *>(get_legacy_config_ptr(DEVINFO_PTR_INTERNAL_MEMORY_MAP));
	m_space_config.m_default_map = reinterpret_cast<const addrmap_token *>(get_legacy_config_ptr(DEVINFO_PTR_DEFAULT_MEMORY_MAP));
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

//**************************************************************************
//  LEGACY IMAGE DEVICE CONFIGURATION
//**************************************************************************
const image_device_type_info legacy_image_device_config_base::m_device_info_array[] =
	{
		{ IO_CARTSLOT,	"cartridge",	"cart" }, /*  0 */
		{ IO_FLOPPY,	"floppydisk",	"flop" }, /*  1 */
		{ IO_HARDDISK,	"harddisk",		"hard" }, /*  2 */
		{ IO_CYLINDER,	"cylinder",		"cyln" }, /*  3 */
		{ IO_CASSETTE,	"cassette",		"cass" }, /*  4 */
		{ IO_PUNCHCARD,	"punchcard",	"pcrd" }, /*  5 */
		{ IO_PUNCHTAPE,	"punchtape",	"ptap" }, /*  6 */
		{ IO_PRINTER,	"printer",		"prin" }, /*  7 */
		{ IO_SERIAL,	"serial",		"serl" }, /*  8 */
		{ IO_PARALLEL,	"parallel",		"parl" }, /*  9 */
		{ IO_SNAPSHOT,	"snapshot",		"dump" }, /* 10 */
		{ IO_QUICKLOAD,	"quickload",	"quik" }, /* 11 */
		{ IO_MEMCARD,	"memcard",		"memc" }, /* 12 */
		{ IO_CDROM,     "cdrom",        "cdrm" }, /* 13 */
		{ IO_MAGTAPE,	"magtape",		"magt" }, /* 14 */
	};

//-------------------------------------------------
//  legacy_image_device_config_base - constructor
//-------------------------------------------------

legacy_image_device_config_base::legacy_image_device_config_base(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock, device_get_config_func get_config)
	: legacy_device_config_base(mconfig, type, tag, owner, clock, get_config),
	  device_config_image_interface(mconfig, *this),
	  m_create_option_guide(NULL),
	  m_formatlist(NULL)
{
}

//-------------------------------------------------
//  device_config_complete - update configuration
//  based on completed device setup
//-------------------------------------------------

void legacy_image_device_config_base::device_config_complete()
{
	const device_config_image_interface *image = NULL;
	int count = 0;
	int index = -1;
    image_device_format **formatptr;
    image_device_format *format;
    formatptr = &m_formatlist;
    int cnt = 0;

	m_type = static_cast<iodevice_t>(get_legacy_config_int(DEVINFO_INT_IMAGE_TYPE));
	m_readable = get_legacy_config_int(DEVINFO_INT_IMAGE_READABLE)!=0;
	m_writeable = get_legacy_config_int(DEVINFO_INT_IMAGE_WRITEABLE)!=0;
	m_creatable = get_legacy_config_int(DEVINFO_INT_IMAGE_CREATABLE)!=0;
	m_must_be_loaded = get_legacy_config_int(DEVINFO_INT_IMAGE_MUST_BE_LOADED)!=0;
	m_reset_on_load = get_legacy_config_int(DEVINFO_INT_IMAGE_RESET_ON_LOAD)!=0;
	m_has_partial_hash = get_legacy_config_int(DEVINFO_FCT_IMAGE_PARTIAL_HASH)!=0;

	m_interface_name = get_legacy_config_string(DEVINFO_STR_IMAGE_INTERFACE);

	m_file_extensions = get_legacy_config_string(DEVINFO_STR_IMAGE_FILE_EXTENSIONS);

	load		 = reinterpret_cast<device_image_load_func>(get_legacy_config_fct(DEVINFO_FCT_IMAGE_LOAD));
	create		 = reinterpret_cast<device_image_create_func>(get_legacy_config_fct(DEVINFO_FCT_IMAGE_CREATE));
	unload		 = reinterpret_cast<device_image_unload_func>(get_legacy_config_fct(DEVINFO_FCT_IMAGE_UNLOAD));
	display	 = reinterpret_cast<device_image_display_func>(get_legacy_config_fct(DEVINFO_FCT_IMAGE_DISPLAY));
	partialhash = reinterpret_cast<device_image_partialhash_func>(get_legacy_config_fct(DEVINFO_FCT_IMAGE_PARTIAL_HASH));
	get_devices = reinterpret_cast<device_image_get_devices_func>(get_legacy_config_fct(DEVINFO_FCT_IMAGE_GET_DEVICES));

	m_create_option_guide = reinterpret_cast<const option_guide *>(get_legacy_config_ptr(DEVINFO_PTR_IMAGE_CREATE_OPTGUIDE));

    int format_count = get_legacy_config_int(DEVINFO_INT_IMAGE_CREATE_OPTCOUNT);

	for (int i = 0; i < format_count; i++)
	{
		// only add if creatable
		if (get_legacy_config_string(DEVINFO_PTR_IMAGE_CREATE_OPTSPEC + i)) {
			// allocate a new format
			format = global_alloc_clear(image_device_format);

			// populate it
			format->m_index       = cnt;
			format->m_name        = get_legacy_config_string(DEVINFO_STR_IMAGE_CREATE_OPTNAME + i);
			format->m_description = get_legacy_config_string(DEVINFO_STR_IMAGE_CREATE_OPTDESC + i);
			format->m_extensions  = get_legacy_config_string(DEVINFO_STR_IMAGE_CREATE_OPTEXTS + i);
			format->m_optspec     = get_legacy_config_string(DEVINFO_PTR_IMAGE_CREATE_OPTSPEC + i);

			// and append it to the list
			*formatptr = format;
			formatptr = &format->m_next;
			cnt++;
		}
	}

	for (bool gotone = device_config_interface::m_machine_config.devicelist.first(image); gotone; gotone = image->next(image))
	{
		if (this == image)
			index = count;
		if (image->image_type_direct() == m_type)
			count++;
	}
	if (count > 1) {
		m_instance_name.printf("%s%d", device_typename(m_type), index + 1);
		m_brief_instance_name.printf("%s%d", device_brieftypename(m_type), index + 1);
	}
	else
	{
		m_instance_name = device_typename(m_type);
		m_brief_instance_name = device_brieftypename(m_type);
	}
	// Override in case of hardcoded values
	if (strlen(get_legacy_config_string(DEVINFO_STR_IMAGE_INSTANCE_NAME))>0) {
		m_instance_name = get_legacy_config_string(DEVINFO_STR_IMAGE_INSTANCE_NAME);
	}
	if (strlen(get_legacy_config_string(DEVINFO_STR_IMAGE_BRIEF_INSTANCE_NAME))>0) {
		m_brief_instance_name = get_legacy_config_string(DEVINFO_STR_IMAGE_BRIEF_INSTANCE_NAME);
	}
}

//-------------------------------------------------
//  find_device_type - search trough list of
//  device types to extact data
//-------------------------------------------------

const image_device_type_info *legacy_image_device_config_base::find_device_type(iodevice_t type)
{
	int i;
	for (i = 0; i < ARRAY_LENGTH(legacy_image_device_config_base::m_device_info_array); i++)
	{
		if (m_device_info_array[i].m_type == type)
			return &m_device_info_array[i];
	}
	return NULL;
}

//-------------------------------------------------
//  device_typename - retrieves device type name
//-------------------------------------------------

const char *legacy_image_device_config_base::device_typename(iodevice_t type)
{
	const image_device_type_info *info = find_device_type(type);
	return (info != NULL) ? info->m_name : NULL;
}

//-------------------------------------------------
//  device_brieftypename - retrieves device
//  brief type name
//-------------------------------------------------

const char *legacy_image_device_config_base::device_brieftypename(iodevice_t type)
{
	const image_device_type_info *info = find_device_type(type);
	return (info != NULL) ? info->m_shortname : NULL;
}

//-------------------------------------------------
//  uses_file_extension - update configuration
//  based on completed device setup
//-------------------------------------------------

bool legacy_image_device_config_base::uses_file_extension(const char *file_extension) const
{
    bool result = FALSE;

	if (file_extension[0] == '.')
        file_extension++;

	/* find the extensions */
	char *ext = strtok((char*)m_file_extensions.cstr(),",");
	while (ext != NULL)
	{
		if (!mame_stricmp(ext, file_extension))
        {
            result = TRUE;
            break;
        }
		ext = strtok (NULL, ",");
	}
    return result;
}

//-------------------------------------------------
//  ~legacy_device_config_base - destructor
//-------------------------------------------------

legacy_image_device_config_base::~legacy_image_device_config_base()
{
    image_device_format **formatptr = &m_formatlist;

	/* free all entries */
	while (*formatptr != NULL)
	{
		image_device_format *entry = *formatptr;
		*formatptr = entry->m_next;
		global_free(entry);
	}
}


//**************************************************************************
//  LIVE LEGACY IMAGE DEVICE
//**************************************************************************

//-------------------------------------------------
//  legacy_image_device_base - constructor
//-------------------------------------------------

legacy_image_device_base::legacy_image_device_base(running_machine &machine, const device_config &config)
	: legacy_device_base(machine, config),
	  device_image_interface(machine, config, *this)
{
}
