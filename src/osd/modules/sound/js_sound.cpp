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


namespace osd {

namespace {

class sound_js : public osd_module, public sound_module
{
public:
	sound_js() : osd_module(OSD_SOUND_PROVIDER, "js"), sound_module()
	{
		m_current_stream_id = 0;
		m_next_stream_id = 1;
		// Query the browser's AudioContext sample rate so MAME can
		// generate audio at the correct rate, avoiding rate-mismatch
		// drift and growing latency in the JS ring buffer.
		m_audio_rate = EM_ASM_INT({
			try {
				var Ctx = typeof AudioContext !== "undefined" ? AudioContext
					: typeof webkitAudioContext !== "undefined" ? webkitAudioContext
					: null;
				if (Ctx) {
					// Construct a temporary context just to read the
					// sample rate. Browsers return the hardware rate.
					var c = new Ctx();
					var rate = c.sampleRate | 0;
					c.close();
					return rate;
				}
			} catch(e) {}
			return 44100; // fallback
		});
	}
	virtual ~sound_js() { }

	virtual int init(osd_interface &osd, const osd_options &options) override
	{
		return 0;
	}

	virtual void exit() override
	{
		m_current_stream_id = 0;
	}

	virtual uint32_t get_generation() override
	{
		return 1;
	}

	virtual osd::audio_info get_information() override
	{
		osd::audio_info result;
		result.m_generation = 1;
		result.m_default_sink = 1;
		result.m_default_source = 0;
		result.m_nodes.resize(1);
		result.m_nodes[0].m_name = "webaudio";
		result.m_nodes[0].m_display_name = "Web Audio";
		result.m_nodes[0].m_id = 1;
		result.m_nodes[0].m_rate.m_default_rate = m_audio_rate;
		result.m_nodes[0].m_rate.m_min_rate = m_audio_rate;
		result.m_nodes[0].m_rate.m_max_rate = m_audio_rate;
		result.m_nodes[0].m_sinks = 2;
		result.m_nodes[0].m_sources = 0;
		result.m_nodes[0].m_port_names.reserve(2);
		result.m_nodes[0].m_port_names.emplace_back("L");
		result.m_nodes[0].m_port_names.emplace_back("R");
		result.m_nodes[0].m_port_positions.reserve(2);
		result.m_nodes[0].m_port_positions.emplace_back(osd::channel_position::FL());
		result.m_nodes[0].m_port_positions.emplace_back(osd::channel_position::FR());
		if (m_current_stream_id)
		{
			result.m_streams.resize(1);
			result.m_streams[0].m_id = m_current_stream_id;
			result.m_streams[0].m_node = 1;
		}
		return result;
	}

	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override
	{
		// currently no support for multiple streams
		if (m_current_stream_id)
			return 0;

		m_current_stream_id = m_next_stream_id++;
		return m_current_stream_id;
	}

	virtual void stream_close(uint32_t id) override
	{
		if (id == m_current_stream_id)
			m_current_stream_id = 0;
	}

	virtual void stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame) override
	{
		if (id != m_current_stream_id)
			return;

		EM_ASM_ARGS(
				{
					// Forward audio stream update on to JS backend implementation.
					jsmame_stream_sink_update($0, $1);
				},
				(unsigned int)buffer,
				samples_this_frame);
	}

private:
	uint32_t m_current_stream_id;
	uint32_t m_next_stream_id;
	uint32_t m_audio_rate;
};

} // anonymous namespace

} // namespace osd

#else // SDLMAME_EMSCRIPTEN

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_js, OSD_SOUND_PROVIDER, "js") } }

#endif // SDLMAME_EMSCRIPTEN

MODULE_DEFINITION(SOUND_JS, osd::sound_js)
