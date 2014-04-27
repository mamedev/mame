//============================================================
//
//  sound.c - SDL implementation of MAME sound routines
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================
#pragma once

#ifndef __SOUND_SDL_H__
#define __SOUND_SDL_H__

#include "osdepend.h"

class sound_sdl : public osd_sound_interface
{
public:
	// construction/destruction
	sound_sdl(const osd_interface &osd);
	virtual ~sound_sdl();
	
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);
};

extern const osd_sound_type OSD_SOUND_SDL;

#endif  /* __SOUND_SDL_H__ */
