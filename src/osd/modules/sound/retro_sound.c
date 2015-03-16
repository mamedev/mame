//============================================================
//
//  retro_sound.c - libretro implementation of MAME sound routines
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "sound_module.h"
#include "modules/osdmodule.h"

// MAME headers
#include "emu.h"
#include "emuopts.h"

#include "libretro.h"

extern int retro_pause;
extern retro_audio_sample_batch_t audio_batch_cb;

//============================================================
//  DEBUGGING
//============================================================

#define LOG_SOUND       0

//  CLASS
//============================================================

class sound_retro : public osd_module, public sound_module
{
public:
	sound_retro()
	: osd_module(OSD_SOUND_PROVIDER, "retro"), sound_module()
	{
	}
	virtual ~sound_retro() { }

	// sound_module

	virtual void update_audio_stream(bool is_throttled, const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);
};

//============================================================
//  update_audio_stream
//============================================================

void sound_retro::update_audio_stream(bool is_throttled, const INT16 *buffer, int samples_this_frame)
{
   if (retro_pause != -1)
      audio_batch_cb(buffer, samples_this_frame);
}



//============================================================
//  set_mastervolume
//============================================================

void sound_retro::set_mastervolume(int _attenuation)
{
}

MODULE_DEFINITION(SOUND_RETRO, sound_retro)
