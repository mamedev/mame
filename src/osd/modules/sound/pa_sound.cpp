// license:BSD-3-Clause
// copyright-holders:O. Galibert
/***************************************************************************

    pa_sound.c

    PortAudio interface.

***************************************************************************/

#include "sound_module.h"

#include "modules/osdmodule.h"

#ifndef NO_USE_PORTAUDIO

#include "modules/lib/osdobj_common.h"
#include "osdcore.h"

#include <portaudio.h>
#include <mutex>
#include <map>

namespace osd {

namespace {

class sound_pa : public osd_module, public sound_module
{
public:
	sound_pa() : osd_module(OSD_SOUND_PROVIDER, "portaudio")
	{
	}
	virtual ~sound_pa() { }

	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override;

	// Sadly, according to the documentation, "Some platforms don't
	// support opening multiple streams on the same device.".  And
	// there doesn't seem to be a way to test for it.  So we're stuck
	// with the lowest level of osd support.

	// In addition, the hotplug branch has been stagnant since 2016,
	// no idea if it will ever be completed and merged.  Meanwhile, no
	// device hotplugging for you.

	virtual bool external_per_channel_volume() override { return false; }
	virtual bool split_streams_per_source() override { return false; }

	virtual uint32_t get_generation() override;
	virtual osd::audio_info get_information() override;
	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual uint32_t stream_source_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual void stream_close(uint32_t id) override;
	virtual void stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame) override;
	virtual void stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame) override;

private:
	struct stream_info {
		sound_pa *m_manager;
		PaStream *m_stream;
		uint32_t m_channels;
		uint32_t m_id;
		uint32_t m_devid;
		abuffer m_buffer;

		stream_info(sound_pa *manager, uint32_t channels, uint32_t id, uint32_t devid) :
			m_manager(manager), m_stream(nullptr), m_channels(channels), m_id(id), m_devid(devid), m_buffer(channels)
		{ }
	};

	osd::audio_info m_info;

	// Used when the structure changes but we're certain the stream callbacks won't be hit.
	// In our case, that means closure callback changing the streams structure.
	std::mutex m_gen_mutex;

	// Used when the stream callbacks need to be protected.
	// In our case, when audio buffers are manipulated on a running stream.
	// There is no real need to have one per stream.
	std::mutex m_stream_mutex;

	std::map<uint32_t, stream_info> m_streams;

	uint32_t m_stream_id;
	float m_audio_latency;

	int stream_callback(stream_info *stream, const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags);
	static int s_stream_callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

	void stream_finished_callback(stream_info *stream);
	static void s_stream_finished_callback(void *userData);
};

int sound_pa::init(osd_interface &osd, osd_options const &options)
{
	// Portaudio does not seem to have any information w.r.t
	// channel positioning, so we'll use the sdl conventions.

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

	static const uint32_t positions[9][9] = {
		{ FC },
		{ FL, FR },
		{ FL, FR, LFE },
		{ FL, FR, BL, BR },
		{ FL, FR, LFE, BL, BR },
		{ FL, FR, FC, LFE, BL, BR },
		{ FL, FR, FC, LFE, BC, SL, SR },
		{ FL, FR, FC, LFE, BL, BR, SL, SR },
		{ FL, FR, AUX, AUX, AUX, AUX, AUX, AUX, AUX }, // pa over alsa emulation of pipewire or pulse is a mess...
	};

	PaError err = Pa_Initialize();
	if(err) {
		osd_printf_error("PortAudio error: %s\n", Pa_GetErrorText(err));
		return 1;
	}

	m_info.m_generation = 1;
	m_info.m_nodes.resize(Pa_GetDeviceCount());
	for(PaDeviceIndex dev = 0; dev != Pa_GetDeviceCount(); dev++) {
		const PaDeviceInfo *di = Pa_GetDeviceInfo(dev);
		const PaHostApiInfo *ai = Pa_GetHostApiInfo(di->hostApi);
		auto &node = m_info.m_nodes[dev];
		node.m_id = dev + 1;
		node.m_rate.m_default_rate = node.m_rate.m_min_rate = node.m_rate.m_max_rate = di->defaultSampleRate;
		node.m_sinks = di->maxOutputChannels;
		node.m_sources = di->maxInputChannels;

		node.m_name = util::string_format("%s: %s", ai->name, di->name);
		node.m_name.erase(std::remove(node.m_name.begin(), node.m_name.end(), '\r'), node.m_name.end());
		node.m_name.erase(std::remove(node.m_name.begin(), node.m_name.end(), '\n'), node.m_name.end());
		node.m_display_name = node.m_name;

		int channels = std::max(di->maxInputChannels, di->maxOutputChannels);
		int index = std::min(channels, 9) - 1;
		for(uint32_t port = 0; port != channels; port++) {
			uint32_t pos = positions[index][std::min(8U, port)];
			node.m_port_names.push_back(posname[pos]);
			node.m_port_positions.push_back(pos3d[pos]);
		}
	}

	auto dc = [](PaDeviceIndex dev) -> int { return dev == paNoDevice ? 0 : dev+1; };
	m_info.m_default_sink = dc(Pa_GetDefaultOutputDevice());
	m_info.m_default_source = dc(Pa_GetDefaultInputDevice());

	m_stream_id = 1;
	m_audio_latency = options.audio_latency() * 20e-3;

	return 0;
}

void sound_pa::exit()
{
	Pa_Terminate();
	m_info.m_nodes.clear();
}

uint32_t sound_pa::get_generation()
{
	std::unique_lock<std::mutex> lock(m_gen_mutex);
	return m_info.m_generation;
}

osd::audio_info sound_pa::get_information()
{
	std::unique_lock<std::mutex> lock(m_gen_mutex);
	return m_info;
}

uint32_t sound_pa::stream_sink_open(uint32_t node, std::string name, uint32_t rate)
{
	std::unique_lock<std::mutex> lock(m_gen_mutex);
	if(node < 1 || node > m_info.m_nodes.size())
		return 0;

	uint32_t id = m_stream_id ++;
	auto si = m_streams.emplace(id, stream_info(this, m_info.m_nodes[node-1].m_sinks, id, node)).first;

	PaStreamParameters op;
	op.device = node - 1;
	op.channelCount = m_info.m_nodes[node-1].m_sinks;
	op.sampleFormat = paInt16;
	op.suggestedLatency = (m_audio_latency > 0.0f) ? m_audio_latency : Pa_GetDeviceInfo(node - 1)->defaultLowOutputLatency;
	op.hostApiSpecificStreamInfo = nullptr;

	PaError err = Pa_OpenStream(&si->second.m_stream, nullptr, &op, rate, paFramesPerBufferUnspecified, 0, s_stream_callback, &si->second);
	if(!err)
		err = Pa_SetStreamFinishedCallback(si->second.m_stream, s_stream_finished_callback);
	if(!err)
		err = Pa_StartStream(si->second.m_stream);
	if(err) {
		osd_printf_error("PortAudio error: %s: %s\n", m_info.m_nodes[node-1].m_display_name, Pa_GetErrorText(err));
		lock.unlock();
		stream_close(id);
		return 0;
	}
	return id;
}

uint32_t sound_pa::stream_source_open(uint32_t node, std::string name, uint32_t rate)
{
	std::unique_lock<std::mutex> lock(m_gen_mutex);
	if(node < 1 || node > m_info.m_nodes.size())
		return 0;

	uint32_t id = m_stream_id ++;
	auto si = m_streams.emplace(id, stream_info(this, m_info.m_nodes[node-1].m_sources, id, node)).first;

	PaStreamParameters ip;
	ip.device = node - 1;
	ip.channelCount = m_info.m_nodes[node-1].m_sources;
	ip.sampleFormat = paInt16;
	ip.suggestedLatency = (m_audio_latency > 0.0f) ? m_audio_latency : Pa_GetDeviceInfo(node - 1)->defaultLowInputLatency;
	ip.hostApiSpecificStreamInfo = nullptr;

	PaError err = Pa_OpenStream(&si->second.m_stream, &ip, nullptr, rate, paFramesPerBufferUnspecified, 0, s_stream_callback, &si->second);
	if(!err)
		err = Pa_SetStreamFinishedCallback(si->second.m_stream, s_stream_finished_callback);
	if(!err)
		err = Pa_StartStream(si->second.m_stream);
	if(err) {
		osd_printf_error("PortAudio error: %s: %s\n", m_info.m_nodes[node-1].m_display_name, Pa_GetErrorText(err));
		lock.unlock();
		stream_close(id);
		return 0;
	}
	return id;
}

void sound_pa::stream_close(uint32_t id)
{
	std::unique_lock<std::mutex> lock(m_gen_mutex);
	auto si = m_streams.find(id);
	if(si == m_streams.end())
		return;
	if(auto *s = si->second.m_stream; s) {
		lock.unlock();
		Pa_CloseStream(s);
	}
	else
		m_streams.erase(si);
}

void sound_pa::stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame)
{
	std::unique_lock<std::mutex> lock(m_gen_mutex);
	std::unique_lock<std::mutex> slock(m_stream_mutex);
	auto si = m_streams.find(id);
	if(si == m_streams.end())
		return;
	si->second.m_buffer.push(buffer, samples_this_frame);
}

void sound_pa::stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame)
{
	std::unique_lock<std::mutex> lock(m_gen_mutex);
	std::unique_lock<std::mutex> slock(m_stream_mutex);
	auto si = m_streams.find(id);
	if(si == m_streams.end())
		return;
	si->second.m_buffer.get(buffer, samples_this_frame);
}

int sound_pa::stream_callback(stream_info *stream, const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags)
{
	std::unique_lock<std::mutex> slock(m_stream_mutex);
	if(output)
		stream->m_buffer.get((int16_t *)output, frameCount);
	if(input)
		stream->m_buffer.push((const int16_t *)input, frameCount);
	return 0;
}

int sound_pa::s_stream_callback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
	stream_info *si = (stream_info *)userData;
	return si->m_manager->stream_callback(si, input, output, frameCount, timeInfo, statusFlags);
}

void sound_pa::stream_finished_callback(stream_info *stream)
{
	std::unique_lock<std::mutex> lock(m_gen_mutex);
	auto si = m_streams.find(stream->m_id);
	if(si == m_streams.end())
		return;
	m_streams.erase(si);
}

void sound_pa::s_stream_finished_callback(void *userData)
{
	stream_info *si = (stream_info *)userData;
	return si->m_manager->stream_finished_callback(si);
}


} // anonymous namespace

} // namespace osd

#else

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_pa, OSD_SOUND_PROVIDER, "portaudio") } }

#endif

MODULE_DEFINITION(SOUND_PORTAUDIO, osd::sound_pa)
