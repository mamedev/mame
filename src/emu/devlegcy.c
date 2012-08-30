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
//  legacy_device_base - constructor
//-------------------------------------------------

legacy_device_base::legacy_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, device_get_config_func get_config)
	: device_t(mconfig, type, "Legacy Device", tag, owner, clock),
	  m_get_config_func(get_config),
	  m_token(NULL)
{
	// set the proper name
	m_name = get_legacy_string(DEVINFO_STR_NAME);
	m_shortname = get_legacy_string(DEVINFO_STR_SHORTNAME);
	m_searchpath = m_shortname;

	// create the token
	int tokenbytes = get_legacy_int(DEVINFO_INT_TOKEN_BYTES);
	if (tokenbytes != 0)
		m_token = global_alloc_array_clear(UINT8, tokenbytes);
}


//-------------------------------------------------
//  ~legacy_device_base - destructor
//-------------------------------------------------

legacy_device_base::~legacy_device_base()
{
	global_free(m_token);
}


//-------------------------------------------------
//  get_legacy_int - return a legacy
//  configuration parameter as an integer
//-------------------------------------------------

INT64 legacy_device_base::get_legacy_int(UINT32 state) const
{
	deviceinfo info = { 0 };
	(*m_get_config_func)(this, state, &info);
	return info.i;
}


//-------------------------------------------------
//  get_legacy_ptr - return a legacy
//  configuration parameter as a pointer
//-------------------------------------------------

void *legacy_device_base::get_legacy_ptr(UINT32 state) const
{
	deviceinfo info = { 0 };
	(*m_get_config_func)(this, state, &info);
	return info.p;
}


//-------------------------------------------------
//  get_legacy_fct - return a legacy
//  configuration parameter as a function pointer
//-------------------------------------------------

genf *legacy_device_base::get_legacy_fct(UINT32 state) const
{
	deviceinfo info = { 0 };
	(*m_get_config_func)(this, state, &info);
	return info.f;
}


//-------------------------------------------------
//  get_legacy_string - return a legacy
//  configuration parameter as a string pointer
//-------------------------------------------------

const char *legacy_device_base::get_legacy_string(UINT32 state) const
{
	deviceinfo info;
	info.s = get_temp_string_buffer();
	(*m_get_config_func)(this, state, &info);
	return info.s;
}


//-------------------------------------------------
//  device_start - called to start up a device
//-------------------------------------------------

void legacy_device_base::device_start()
{
	device_start_func start_func = reinterpret_cast<device_start_func>(get_legacy_fct(DEVINFO_FCT_START));
	assert(start_func != NULL);
	(*start_func)(this);
}


//-------------------------------------------------
//  device_reset - called to reset a device
//-------------------------------------------------

void legacy_device_base::device_reset()
{
	device_reset_func reset_func = reinterpret_cast<device_reset_func>(get_legacy_fct(DEVINFO_FCT_RESET));
	if (reset_func != NULL)
		(*reset_func)(this);
}


//-------------------------------------------------
//  device_stop - called to stop a device
//-------------------------------------------------

void legacy_device_base::device_stop()
{
	if (started())
	{
		device_stop_func stop_func = reinterpret_cast<device_stop_func>(get_legacy_fct(DEVINFO_FCT_STOP));
		if (stop_func != NULL)
			(*stop_func)(this);
	}
}


//**************************************************************************
//  LEGACY SOUND DEVICE
//**************************************************************************

//-------------------------------------------------
//  legacy_sound_device_base - constructor
//-------------------------------------------------

legacy_sound_device_base::legacy_sound_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, device_get_config_func get_config)
	: legacy_device_base(mconfig, type, tag, owner, clock, get_config),
	  device_sound_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void legacy_sound_device_base::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("legacy_sound_device_base::sound_stream_update called; not applicable to legacy sound devices\n");
}
