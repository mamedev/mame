// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    none.h

    Dummy sound interface.

*******************************************************************c********/

#pragma once

#ifndef __SOUND_NONE_H__
#define __SOUND_NONE_H__

#include "osdepend.h"

class sound_none : public osd_sound_interface
{
public:
	// construction/destruction
	sound_none(const osd_interface &osd);
	virtual ~sound_none() { }
	
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame) { }
	virtual void set_mastervolume(int attenuation) { }
};

extern const osd_sound_type OSD_SOUND_NONE;

#endif  /* __SOUND_NONE_H__ */
