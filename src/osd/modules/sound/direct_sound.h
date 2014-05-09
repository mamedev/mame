// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  sound.c - Win32 implementation of MAME sound routines
//
//============================================================
#pragma once

#ifndef __SOUND_DSOUND_H__
#define __SOUND_DSOUND_H__

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

// undef WINNT for dsound.h to prevent duplicate definition
#undef WINNT
#include <dsound.h>
#undef interface

#include "osdepend.h"

class sound_direct_sound : public osd_sound_interface
{
public:
	// construction/destruction
	sound_direct_sound(const osd_interface &osd);
	virtual ~sound_direct_sound();
	
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);

	HRESULT      dsound_init();
	void         dsound_kill();
	HRESULT      dsound_create_buffers();
	void         dsound_destroy_buffers();
	void 		 copy_sample_data(const INT16 *data, int bytes_to_copy);
private:	
};

extern const osd_sound_type OSD_SOUND_DIRECT_SOUND;

#endif  /* __SOUND_DSOUND_H__ */
