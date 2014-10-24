/***************************************************************************

    retro_sound.h

    Dummy sound interface.

*******************************************************************c********/

#pragma once

#ifndef __SOUND_RETRO_H__
#define __SOUND_RETRO_H__

#include "osdepend.h"

class sound_retro : public osd_sound_interface
{
public:
	// construction/destruction
	sound_retro(const osd_interface &osd);
	virtual ~sound_retro() { }
	
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);
};

extern const osd_sound_type OSD_SOUND_RETRO;

#endif  /* __SOUND_RETRO_H__ */
