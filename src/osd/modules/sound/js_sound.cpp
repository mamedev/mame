// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Katelyn Gadd
/***************************************************************************

    js_sound.c

    Shim for native JavaScript sound interface implementations (Emscripten only).

****************************************************************************/

#include "sound_module.h"
#include "modules/osdmodule.h"

#if (defined(SDLMAME_EMSCRIPTEN))

#include "emscripten.h"

class sound_js : public osd_module, public sound_module
{
public:

	sound_js()
	: osd_module(OSD_SOUND_PROVIDER, "js"), sound_module()
	{
	}
	virtual ~sound_js() { }

	virtual int init(const osd_options &options) { return 0; }
	virtual void exit() { }

	// sound_module

	virtual void update_audio_stream(bool is_throttled, const INT16 *buffer, int samples_this_frame)
	{
		EM_ASM_ARGS({
		// Forward audio stream update on to JS backend implementation.
		jsmame_update_audio_stream($0, $1);
		}, (unsigned int)buffer, samples_this_frame);
	}
	virtual void set_mastervolume(int attenuation)
	{
		EM_ASM_ARGS({
		// Forward volume update on to JS backend implementation.
		jsmame_set_mastervolume($0);
		}, attenuation);
	}

};

#else /* SDLMAME_UNIX */
	MODULE_NOT_SUPPORTED(sound_js, OSD_SOUND_PROVIDER, "js")
#endif

MODULE_DEFINITION(SOUND_JS, sound_js)
