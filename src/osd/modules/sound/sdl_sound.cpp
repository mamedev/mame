// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sound.c - SDL implementation of MAME sound routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "sound_module.h"

#include "modules/osdmodule.h"

#if (defined(OSD_SDL) || defined(USE_SDL_SOUND))

#include "modules/lib/osdobj_common.h"
#include "osdcore.h"

// standard sdl header
#include <SDL2/SDL.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <map>


namespace osd {

namespace {

class sound_sdl : public osd_module, public sound_module
{
public:
	sound_sdl() :
		osd_module(OSD_SOUND_PROVIDER, "sdl"), sound_module()
	{
	}

	virtual ~sound_sdl() { }

	virtual int init(osd_interface &osd, const osd_options &options) override;
	virtual void exit() override;

	virtual bool external_per_channel_volume() override { return false; }
	virtual bool split_streams_per_source() override { return false; }

	virtual uint32_t get_generation() override;
	virtual osd::audio_info get_information() override;
	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual void stream_close(uint32_t id) override;
	virtual void stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame) override;

private:
	struct device_info {
		std::string m_name;
		int m_freq;
		uint8_t m_channels;
		device_info(const char *name, int freq, uint8_t channels) : m_name(name), m_freq(freq), m_channels(channels) {}
	};

	struct stream_info {
		uint32_t m_id;
		SDL_AudioDeviceID m_sdl_id;
		abuffer m_buffer;
		stream_info(uint32_t id, uint8_t channels) : m_id(id), m_sdl_id(0), m_buffer(channels) {}
	};

	std::vector<device_info> m_devices;
	uint32_t m_default_sink;
	uint32_t m_stream_next_id;

	std::map<uint32_t, std::unique_ptr<stream_info>> m_streams;

	static void sink_callback(void *userdata, Uint8 *stream, int len);
};

//============================================================
//  sound_sdl::init
//============================================================

int sound_sdl::init(osd_interface &osd, const osd_options &options)
{
	m_stream_next_id = 1;

	if(SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		osd_printf_error("Could not initialize SDL %s\n", SDL_GetError());
		return -1;
	}

	osd_printf_verbose("Audio: Start initialization\n");
	char const *const audio_driver = SDL_GetCurrentAudioDriver();
	osd_printf_verbose("Audio: Driver is %s\n", audio_driver ? audio_driver : "not initialized");

	// Capture is not implemented in SDL2, and the enumeration
	// interface is different in SDL3
	int dev_count = SDL_GetNumAudioDevices(0);
	for(int i=0; i != dev_count; i++) {
		SDL_AudioSpec spec;
		const char *name = SDL_GetAudioDeviceName(i, 0);
		int err = SDL_GetAudioDeviceSpec(i, 0, &spec);
		if(!err)
			m_devices.emplace_back(name, spec.freq, spec.channels);
	}
	char *def_name;
	SDL_AudioSpec def_spec;
	if(!SDL_GetDefaultAudioInfo(&def_name, &def_spec, 0)) {
		uint32_t idx;
		for(idx = 0; idx != m_devices.size() && m_devices[idx].m_name != def_name; idx++);
		if(idx == m_devices.size())
			m_devices.emplace_back(def_name, def_spec.freq, def_spec.channels);
		m_default_sink = idx+1;
		SDL_free(def_name);
	} else
		m_default_sink = 0;
	return 0;
}

void sound_sdl::exit()
{
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

uint32_t sound_sdl::get_generation()
{
	// sdl2 is not dynamic w.r.t devices
	return 1;
}

osd::audio_info sound_sdl::get_information()
{
	enum { FL, FR, FC, LFE, BL, BR, BC, SL, SR };
	static const char *const posname[9] = { "FL", "FR", "FC", "LFE", "BL", "BR", "BC", "SL", "SR" };

	static std::array<double, 3> pos3d[9] = {
		{ -0.2,  0.0,  1.0 },
		{  0.2,  0.0,  1.0 },
		{  0.0,  0.0,  1.0 },
		{  0.0, -0.5,  1.0 },
		{ -0.2,  0.0, -0.5 },
		{  0.2,  0.0, -0.5 },
		{  0.0,  0.0, -0.5 },
		{ -0.2,  0.0,  0.0 },
		{  0.2,  0.0,  0.0 },
	};		

	static const uint32_t positions[8][8] = {
		{ FC },
		{ FL, FR },
		{ FL, FR, LFE },
		{ FL, FR, BL, BR },
		{ FL, FR, LFE, BL, BR },
		{ FL, FR, FC, LFE, BL, BR },
		{ FL, FR, FC, LFE, BC, SL, SR },
		{ FL, FR, FC, LFE, BL, BR, SL, SR }
	};

	osd::audio_info result;
	result.m_nodes.resize(m_devices.size());
	result.m_default_sink = m_default_sink;
	result.m_default_source = 0;
	result.m_generation = 1;
	for(uint32_t node = 0; node != m_devices.size(); node++) {
		result.m_nodes[node].m_name = m_devices[node].m_name;
		result.m_nodes[node].m_id = node + 1;
		uint32_t freq = m_devices[node].m_freq;
		result.m_nodes[node].m_rate = audio_rate_range{ freq, freq, freq };
		result.m_nodes[node].m_sinks = m_devices[node].m_channels;
		for(uint32_t port = 0; port != m_devices[node].m_channels; port++) {
			uint32_t pos = positions[m_devices[node].m_channels-1][port];
			result.m_nodes[node].m_port_names.push_back(posname[pos]);
			result.m_nodes[node].m_port_positions.push_back(pos3d[pos]);
		}
	}
	return result;
}

uint32_t sound_sdl::stream_sink_open(uint32_t node, std::string name, uint32_t rate)
{
	device_info &dev = m_devices[node-1];
	std::unique_ptr<stream_info> stream = std::make_unique<stream_info>(m_stream_next_id ++, dev.m_channels);

	SDL_AudioSpec dspec, ospec;
	dspec.freq = rate;
	dspec.format = AUDIO_S16SYS;
	dspec.channels = dev.m_channels;
	dspec.samples = 512;
	dspec.callback = sink_callback;
	dspec.userdata = stream.get();

	stream->m_sdl_id = SDL_OpenAudioDevice(dev.m_name.c_str(), 0, &dspec, &ospec, 0);
	if(!stream->m_sdl_id)
		return 0;
	SDL_PauseAudioDevice(stream->m_sdl_id, 0);
	uint32_t id = stream->m_id;
	m_streams[stream->m_id] = std::move(stream);
	return id;
}

void sound_sdl::stream_close(uint32_t id)
{
	auto si = m_streams.find(id);
	if(si == m_streams.end())
		return;
	SDL_CloseAudioDevice(si->second->m_sdl_id);
	m_streams.erase(si);
}

void sound_sdl::stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame)
{
	auto si = m_streams.find(id);
	if(si == m_streams.end())
		return;
	stream_info *stream = si->second.get();
	SDL_LockAudioDevice(stream->m_sdl_id);
	stream->m_buffer.push(buffer, samples_this_frame);
	SDL_UnlockAudioDevice(stream->m_sdl_id);
}

void sound_sdl::sink_callback(void *userdata, uint8_t *data, int len)
{
	stream_info *stream = reinterpret_cast<stream_info *>(userdata);
	stream->m_buffer.get((int16_t *)data, len / 2 / stream->m_buffer.channels());
}

} // anonymous namespace

} // namespace osd


#else // (defined(OSD_SDL) || defined(USE_SDL_SOUND))

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_sdl, OSD_SOUND_PROVIDER, "sdl") } }

#endif

MODULE_DEFINITION(SOUND_SDL, osd::sound_sdl)
