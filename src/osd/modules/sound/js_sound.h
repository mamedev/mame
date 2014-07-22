// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Katelyn Gadd
/***************************************************************************

    js_sound.h

    Shim for native JavaScript sound interface implementations (Emscripten only).

*******************************************************************c********/

#pragma once

#ifndef __SOUND_JS_H__
#define __SOUND_JS_H__

#include "osdepend.h"

class sound_js : public osd_sound_interface
{
public:
	// construction/destruction
	sound_js(const osd_interface &osd);
	virtual ~sound_js() { }

	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);
};

extern const osd_sound_type OSD_SOUND_JS;

#endif  /* __SOUND_JS_H__ */
