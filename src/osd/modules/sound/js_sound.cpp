// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Katelyn Gadd
/***************************************************************************

    js_sound.c

    Shim for native JavaScript sound interface implementations (Emscripten only).

****************************************************************************/

#include "sound_module.h"
#include "modules/osdmodule.h"

#if defined(SDLMAME_EMSCRIPTEN)

#include "emscripten.h"

class sound_js : public osd_module, public sound_module
{
public:

	sound_js() : osd_module(OSD_SOUND_PROVIDER, "js"), sound_module()
	{
	}
	virtual ~sound_js() { }

	virtual int init(osd_interface &osd, const osd_options &options) { return 0; }
	virtual void exit() { }

	virtual void stream_sink_update(uint32_t, const int16_t *buffer, int samples_this_frame)
	{
		EM_ASM_ARGS(
				{
					// Forward audio stream update on to JS backend implementation.
					jsmame_stream_sink_update($0, $1);
				},
				(unsigned int)buffer,
				samples_this_frame);
	}
};

#else // SDLMAME_EMSCRIPTEN
	MODULE_NOT_SUPPORTED(sound_js, OSD_SOUND_PROVIDER, "js")
#endif // SDLMAME_EMSCRIPTEN

MODULE_DEFINITION(SOUND_JS, sound_js)
