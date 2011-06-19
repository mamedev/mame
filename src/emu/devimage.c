/***************************************************************************

    devimage.c

    Legacy image device helpers.

****************************************************************************

    Copyright Miodrag Milanovic
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
//  LEGACY IMAGE DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  legacy_device_base - destructor
//-------------------------------------------------

legacy_image_device_base::~legacy_image_device_base()
{
}

//**************************************************************************
//  LIVE LEGACY IMAGE DEVICE
//**************************************************************************

//-------------------------------------------------
//  legacy_image_device_base - constructor
//-------------------------------------------------

legacy_image_device_base::legacy_image_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, device_get_config_func get_config)
	: legacy_device_base(mconfig, type, tag, owner, clock, get_config),
	  device_image_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_config_complete - update configuration
//  based on completed device setup
//-------------------------------------------------

void legacy_image_device_base::device_config_complete()
{
    image_device_format **formatptr;
    image_device_format *format;
    formatptr = &m_formatlist;
    int cnt = 0;

    int format_count = get_legacy_int(DEVINFO_INT_IMAGE_CREATE_OPTCOUNT);

	for (int i = 0; i < format_count; i++)
	{
		// only add if creatable
		if (get_legacy_string(DEVINFO_PTR_IMAGE_CREATE_OPTSPEC + i)) {
			// allocate a new format
			format = global_alloc_clear(image_device_format);

			// populate it
			format->m_index       = cnt;
			format->m_name        = get_legacy_string(DEVINFO_STR_IMAGE_CREATE_OPTNAME + i);
			format->m_description = get_legacy_string(DEVINFO_STR_IMAGE_CREATE_OPTDESC + i);
			format->m_extensions  = get_legacy_string(DEVINFO_STR_IMAGE_CREATE_OPTEXTS + i);
			format->m_optspec     = get_legacy_string(DEVINFO_PTR_IMAGE_CREATE_OPTSPEC + i);

			// and append it to the list
			*formatptr = format;
			formatptr = &format->m_next;
			cnt++;
		}
	}

	update_names();

	// Override in case of hardcoded values
	if (strlen(get_legacy_string(DEVINFO_STR_IMAGE_INSTANCE_NAME))>0) {
		m_instance_name = get_legacy_string(DEVINFO_STR_IMAGE_INSTANCE_NAME);
	}
	if (strlen(get_legacy_string(DEVINFO_STR_IMAGE_BRIEF_INSTANCE_NAME))>0) {
		m_brief_instance_name = get_legacy_string(DEVINFO_STR_IMAGE_BRIEF_INSTANCE_NAME);
	}
}


bool legacy_image_device_base::call_load()
{
	device_image_load_func func = reinterpret_cast<device_image_load_func>(get_legacy_fct(DEVINFO_FCT_IMAGE_LOAD));
	if (func) {
		return (*func)(*this);
	} else {
		return FALSE;
	}
}

bool legacy_image_device_base::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	device_image_softlist_load_func func = reinterpret_cast<device_image_softlist_load_func>(get_legacy_fct(DEVINFO_FCT_IMAGE_SOFTLIST_LOAD));
	if (func) {
		return (*func)(*this,swlist,swname,start_entry);
	} else {
		return FALSE;
	}
}

bool legacy_image_device_base::call_create(int format_type, option_resolution *format_options)
{
	device_image_create_func func = reinterpret_cast<device_image_create_func>(get_legacy_fct(DEVINFO_FCT_IMAGE_CREATE));
	if (func) {
		return (*func)(*this,format_type,format_options);
	} else {
		return FALSE;
	}
}

void legacy_image_device_base::call_unload()
{
	device_image_unload_func func = reinterpret_cast<device_image_unload_func>(get_legacy_fct(DEVINFO_FCT_IMAGE_UNLOAD));
	if (func) (*func)(*this);
}

void legacy_image_device_base::call_display()
{
	device_image_display_func func = reinterpret_cast<device_image_display_func>(get_legacy_fct(DEVINFO_FCT_IMAGE_DISPLAY));
	if (func) (*func)(*this);
}

void legacy_image_device_base::call_display_info()
{
	device_image_display_info_func func = reinterpret_cast<device_image_display_info_func>(get_legacy_fct(DEVINFO_FCT_IMAGE_DISPLAY_INFO));
	if (func) (*func)(*this);
}

void legacy_image_device_base::call_get_devices()
{
	device_image_get_devices_func func = reinterpret_cast<device_image_get_devices_func>(get_legacy_fct(DEVINFO_FCT_IMAGE_GET_DEVICES));
	if (func) (*func)(*this);
}

device_image_partialhash_func legacy_image_device_base::get_partial_hash() const
{
	return reinterpret_cast<device_image_partialhash_func>(get_legacy_fct(DEVINFO_FCT_IMAGE_PARTIAL_HASH));
}

void *legacy_image_device_base::get_device_specific_call()
{
	return (void*) get_legacy_fct(DEVINFO_FCT_DEVICE_SPECIFIC);
}
