// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Katelyn Gadd
/***************************************************************************

    js_sound.c

    Shim for native JavaScript sound interface implementations (Emscripten only).

*******************************************************************c********/


#include "js_sound.h"
#include "emscripten.h"

//-------------------------------------------------
//  sound_js - constructor
//-------------------------------------------------
sound_js::sound_js(const osd_interface &osd)
	: osd_sound_interface(osd)
{
}

void sound_js::update_audio_stream(const INT16 *buffer, int samples_this_frame)
{
	EM_ASM_ARGS({
	// Forward audio stream update on to JS backend implementation.
	jsmess_update_audio_stream($0, $1);
	}, (unsigned int)buffer, samples_this_frame);
}

void sound_js::set_mastervolume(int attenuation)
{
	EM_ASM_ARGS({
	// Forward volume update on to JS backend implementation.
	jsmess_set_mastervolume($0);
	}, attenuation);
}

const osd_sound_type OSD_SOUND_JS = &osd_sound_creator<sound_js>;
