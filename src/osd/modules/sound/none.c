// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    none.c

    Dummy sound interface.

*******************************************************************c********/

#include "sound_module.h"
#include "modules/osdmodule.h"

class sound_none : public osd_module, public sound_module
{
public:
	sound_none()
	: osd_module(OSD_SOUND_PROVIDER, "none"), sound_module()
	{
	}
	virtual ~sound_none() { }

	virtual int init(const osd_options &options) { return 0; }
	virtual void exit() { }

	// sound_module

	virtual void update_audio_stream(bool is_throttled, const INT16 *buffer, int samples_this_frame) { }
	virtual void set_mastervolume(int attenuation) { }

};

MODULE_DEFINITION(SOUND_NONE, sound_none)
