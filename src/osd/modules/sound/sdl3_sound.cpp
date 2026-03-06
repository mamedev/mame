// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  sdl3_sound.cpp - SDL3+ implementation of MAME sound routines
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include "sound_module.h"

#include "modules/osdmodule.h"

#if (defined(OSD_SDL) || defined(USE_SDL_SOUND)) && defined(SDLMAME_SDL3)

#include "modules/lib/osdobj_common.h"
#include "osdcore.h"

// standard sdl header
#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <map>


namespace osd {

namespace {

class sound_sdl3 : public osd_module, public sound_module
{
public:
	sound_sdl3() :
		osd_module(OSD_SOUND_PROVIDER, "sdl"), sound_module()
	{
	}

	virtual ~sound_sdl3() { }

	virtual int init(osd_interface &osd, const osd_options &options) override;
	virtual void exit() override;

	virtual bool external_per_channel_volume() override { return false; }
	virtual bool split_streams_per_source() override { return true; }

	virtual uint32_t get_generation() override;
	virtual osd::audio_info get_information() override;
	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual uint32_t stream_source_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual void stream_close(uint32_t id) override;
	virtual void stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame) override;
	virtual void stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame) override;

private:
	struct device_info {
		SDL_AudioDeviceID m_device_id;
		std::string m_name;
		int m_freq;
		uint8_t m_channels;
		bool m_issource;
		bool m_def;
		device_info(const SDL_AudioDeviceID device_id, const char *name, int freq, uint8_t channels, bool source, bool def = false) :
			m_device_id(device_id),
			m_name(name),
			m_freq(freq),
			m_channels(channels),
			m_issource(source),
			m_def(def)
		{
		}
	};

	struct stream_info {
		uint32_t m_id;
		SDL_AudioDeviceID m_sdl_id;
		SDL_AudioStream *m_sdl_stream;
		abuffer m_buffer;
		stream_info(uint32_t id, uint8_t channels) : m_id(id), m_sdl_id(0), m_buffer(channels) {}
	};

	std::vector<device_info> m_devices;
	uint32_t m_default_sink, m_default_source;
	uint32_t m_stream_next_id;
	osd::audio_info m_deviceinfo;

	std::map<uint32_t, std::unique_ptr<stream_info>> m_streams;

	static void sink_callback(void *userdata, Uint8 *stream, int len);
};

//============================================================
//  sound_sdl3::init
//============================================================

int sound_sdl3::init(osd_interface &osd, const osd_options &options)
{
	m_stream_next_id = 1;

	if(!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		osd_printf_error("Could not initialize SDL %s\n", SDL_GetError());
		return -1;
	}

	osd_printf_verbose("SDL Audio: Start initialization\n");
	char const *const audio_driver = SDL_GetCurrentAudioDriver();
	osd_printf_verbose("SDL Audio: Driver is %s\n", audio_driver ? audio_driver : "not initialized");

	if(options.audio_latency() > 0.0f)
		osd_printf_verbose("SDL Audio: %s module does not support audio_latency option\n", name());

	int dev_count = 0;
	SDL_AudioSpec spec;
	SDL_AudioDeviceID *devices = SDL_GetAudioPlaybackDevices(&dev_count);

	if (dev_count) {
		// add default sink device
		SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr);
		// if the device reports 0 channels, force it to stereo as a fallback
		if (spec.channels == 0) {
			spec.channels = 2;
		}
		m_devices.emplace_back(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, "Default", spec.freq, spec.channels, false, true);
		m_default_sink = m_devices.size();
		osd_printf_verbose("SDL Audio: Sink device %d: device 'SDL default sink device'\n", m_default_sink);
	}

	for(int i=0; i != dev_count; i++) {
		const char *const name = SDL_GetAudioDeviceName(devices[i]);
		const bool success = SDL_GetAudioDeviceFormat(devices[i], &spec, nullptr);

		// if the device reports 0 channels, force it to stereo as a fallback
		if (spec.channels == 0) {
			spec.channels = 2;
		}

		if(success) {
			m_devices.emplace_back(devices[i], name, spec.freq, spec.channels, false);

			osd_printf_verbose("SDL Audio: Sink device %d: device '%s' freq %d channels %d (SDL ID %d)\n", m_devices.size(), name, spec.freq, spec.channels, devices[i]);
		}
	}

	SDL_free(devices);

	devices = SDL_GetAudioRecordingDevices(&dev_count);

	if (dev_count) {
		// add default device
		SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &spec, nullptr);
		// if the device reports 0 channels, assume mono as a good microphone fallback
		if (spec.channels == 0) {
			spec.channels = 1;
		}
		m_devices.emplace_back(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, "Default", spec.freq, spec.channels, true, true);
		m_default_source = m_devices.size();
		osd_printf_verbose("SDL Audio: Source device %d: device 'SDL default source device'\n", m_default_source);
	}

	for (int i = 0; i != dev_count; i++)
	{
		const char *const name = SDL_GetAudioDeviceName(devices[i]);
		const bool success = SDL_GetAudioDeviceFormat(devices[i], &spec, nullptr);

		// if the device reports 0 channels, assume mono as a good microphone fallback
		if (spec.channels == 0)
		{
			spec.channels = 1;
		}

		if (success) {
			m_devices.emplace_back(devices[i], name, spec.freq, spec.channels, true);
			osd_printf_verbose("SDL Audio: Source device %d: device '%s' freq %d channels %d (SDL ID %d)\n", m_devices.size(), name, spec.freq, spec.channels, devices[i]);
		}
	}
	SDL_free(devices);
	return 0;
}

void sound_sdl3::exit()
{
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	m_devices.clear();
}

uint32_t sound_sdl3::get_generation()
{
	return 1;
}

osd::audio_info sound_sdl3::get_information()
{
	enum { FL, FR, FC, LFE, BL, BR, BC, SL, SR, AUX };
	static const char *const posname[10] = { "FL", "FR", "FC", "LFE", "BL", "BR", "BC", "SL", "SR", "AUX" };

	static const osd::channel_position pos3d[10] = {
		osd::channel_position::FL(),
		osd::channel_position::FR(),
		osd::channel_position::FC(),
		osd::channel_position::LFE(),
		osd::channel_position::RL(),
		osd::channel_position::RR(),
		osd::channel_position::RC(),
		osd::channel_position(-0.2,  0.0,  0.0),
		osd::channel_position( 0.2,  0.0,  0.0),
		osd::channel_position::ONREQ()
	};

	static const uint32_t positions[8][9] = {
		{ FC },
		{ FL, FR },
		{ FL, FR, LFE },
		{ FL, FR, BL, BR },
		{ FL, FR, LFE, BL, BR },
		{ FL, FR, FC, LFE, BL, BR },
		{ FL, FR, FC, LFE, BC, SL, SR },
		{ FL, FR, FC, LFE, BL, BR, SL, SR, AUX }
	};

	m_deviceinfo.m_nodes.clear();
	m_deviceinfo.m_nodes.resize(m_devices.size());
	m_deviceinfo.m_default_sink = m_default_sink;
	m_deviceinfo.m_default_source = 0;
	m_deviceinfo.m_generation = 1;
	for (uint32_t node = 0; node < m_devices.size(); node++) {
		m_deviceinfo.m_nodes[node].m_name = m_devices[node].m_name;
		m_deviceinfo.m_nodes[node].m_display_name = m_devices[node].m_name;
		m_deviceinfo.m_nodes[node].m_id = node + 1;
		uint32_t freq = m_devices[node].m_freq;
		m_deviceinfo.m_nodes[node].m_rate = audio_rate_range{ freq, freq, freq };
		if (m_devices[node].m_issource) {
			m_deviceinfo.m_nodes[node].m_sources = m_devices[node].m_channels;
		} else {
			m_deviceinfo.m_nodes[node].m_sinks = m_devices[node].m_channels;
		}
		int channels = m_devices[node].m_channels;
		int index = std::min(channels, 8) - 1;
		for(uint32_t port = 0; port != channels; port++) {
			uint32_t pos = positions[index][std::min(8U, port)];
			m_deviceinfo.m_nodes[node].m_port_names.push_back(posname[pos]);
			m_deviceinfo.m_nodes[node].m_port_positions.push_back(pos3d[pos]);
		}
	}
	return m_deviceinfo;
}

uint32_t sound_sdl3::stream_sink_open(uint32_t node, std::string name, uint32_t rate)
{
	const int devnode = node - 1;
	const SDL_AudioDeviceID device_id = m_devices[devnode].m_device_id;
	SDL_AudioSpec dspec;

	dspec.freq = m_devices[devnode].m_freq;
	dspec.format = SDL_AUDIO_S16;
	dspec.channels = m_devices[devnode].m_channels;

	std::unique_ptr<stream_info> stream = std::make_unique<stream_info>(m_stream_next_id ++, dspec.channels);

	stream->m_sdl_id = SDL_OpenAudioDevice(device_id, &dspec);
	if(!stream->m_sdl_id) {
		osd_printf_error("SDL Audio: Could not open audio device %s\n", SDL_GetError());
		return 0;
	}

	stream->m_sdl_stream = SDL_CreateAudioStream(&dspec, nullptr);
	if (!stream->m_sdl_stream) {
		osd_printf_error("SDL Audio: Could not create audio stream %s\n", SDL_GetError());
		SDL_CloseAudioDevice(stream->m_sdl_id);
		return 0;
	}

	if (!SDL_BindAudioStream(stream->m_sdl_id, stream->m_sdl_stream)) {
		osd_printf_error("SDL Audio: Could not bind audio stream %s\n", SDL_GetError());
		SDL_DestroyAudioStream(stream->m_sdl_stream);
		SDL_CloseAudioDevice(stream->m_sdl_id);
		return 0;
	}

	uint32_t id = stream->m_id;
	m_streams[stream->m_id] = std::move(stream);
	osd_printf_verbose("SDL Audio: Opened sink stream id %d on device %d at rate %d\n", id, device_id, rate);
	return id;
}

uint32_t sound_sdl3::stream_source_open(uint32_t node, std::string name, uint32_t rate)
{
	const int devnode = node - 1;
	const SDL_AudioDeviceID device_id = m_devices[devnode].m_device_id;
	SDL_AudioSpec dspec;

	dspec.freq = m_devices[devnode].m_freq;
	dspec.format = SDL_AUDIO_S16;
	dspec.channels = m_devices[devnode].m_channels;

	std::unique_ptr<stream_info> stream = std::make_unique<stream_info>(m_stream_next_id++, dspec.channels);

	printf("opening source device %d at rate %d channels %d\n", device_id, dspec.freq, dspec.channels);

	stream->m_sdl_id = SDL_OpenAudioDevice(device_id, &dspec);
	if (!stream->m_sdl_id)
	{
		osd_printf_error("SDL Audio: Could not open audio device %s\n", SDL_GetError());
		return 0;
	}

	stream->m_sdl_stream = SDL_CreateAudioStream(nullptr, &dspec);
	if (!stream->m_sdl_stream)
	{
		osd_printf_error("SDL Audio: Could not create audio stream %s\n", SDL_GetError());
		SDL_CloseAudioDevice(stream->m_sdl_id);
		return 0;
	}

	if (!SDL_BindAudioStream(stream->m_sdl_id, stream->m_sdl_stream))
	{
		osd_printf_error("SDL Audio: Could not bind audio stream %s\n", SDL_GetError());
		SDL_DestroyAudioStream(stream->m_sdl_stream);
		SDL_CloseAudioDevice(stream->m_sdl_id);
		return 0;
	}

	uint32_t id = stream->m_id;
	m_streams[stream->m_id] = std::move(stream);
	osd_printf_verbose("SDL Audio: Opened source stream id %d on device %d at rate %d\n", id, device_id, rate);
	return id;
}

void sound_sdl3::stream_close(uint32_t id)
{
	osd_printf_verbose("SDL Audio: Closing stream id %d\n", id);

	auto si = m_streams.find(id);
	if(si == m_streams.end())
		return;
	SDL_UnbindAudioStream(si->second->m_sdl_stream);
	SDL_DestroyAudioStream(si->second->m_sdl_stream);
	SDL_CloseAudioDevice(si->second->m_sdl_id);
	m_streams.erase(si);
}

void sound_sdl3::stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame)
{
	auto si = m_streams.find(id);
	if(si == m_streams.end())
		return;
	stream_info *stream = si->second.get();
	SDL_PutAudioStreamData(stream->m_sdl_stream, (void *)buffer, samples_this_frame * sizeof(int16_t) * stream->m_buffer.channels());
}

void sound_sdl3::stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame)
{
	auto si = m_streams.find(id);
	if (si == m_streams.end())
		return;
	stream_info *stream = si->second.get();
	SDL_GetAudioStreamData(stream->m_sdl_stream, (void *)buffer, samples_this_frame * sizeof(int16_t) * stream->m_buffer.channels());
}

} // anonymous namespace

} // namespace osd

#else // (defined(OSD_SDL) || defined(USE_SDL_SOUND)) && defined(SDLMAME_SDL3)

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_sdl3, OSD_SOUND_PROVIDER, "sdl") } }

#endif

MODULE_DEFINITION(SOUND_SDL3, osd::sound_sdl3)

