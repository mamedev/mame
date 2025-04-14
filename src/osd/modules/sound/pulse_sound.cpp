// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    pulse_sound.c

    PulseAudio interface.

*******************************************************************c********/

#include "sound_module.h"
#include "modules/osdmodule.h"

#ifndef NO_USE_PULSEAUDIO

#define GNU_SOURCE

#include <pulse/pulseaudio.h>
#include <map>

#include "modules/lib/osdobj_common.h"

class sound_pulse : public osd_module, public sound_module
{
public:
	sound_pulse()
		: osd_module(OSD_SOUND_PROVIDER, "pulse"), sound_module()
	{
	}
	virtual ~sound_pulse() { }

	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override;

	virtual bool external_per_channel_volume() override { return false; }
	virtual bool split_streams_per_source() override { return true; }

	virtual uint32_t get_generation() override;
	virtual osd::audio_info get_information() override;
	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual void stream_set_volumes(uint32_t id, const std::vector<float> &db) override;
	virtual void stream_close(uint32_t id) override;
	virtual void stream_sink_update(uint32_t, const int16_t *buffer, int samples_this_frame) override;

private:
	struct position_info {
		pa_channel_position_t m_position;
		std::array<double, 3> m_coords;
	};

	static const position_info position_infos[];
	static const char *const typenames[];
	enum { AREC, APLAY };

	struct node_info {
		sound_pulse *m_pulse;
		uint32_t m_id, m_osdid;
		int m_type;
		std::string m_name, m_desc;

		// Audio node info
		std::vector<pa_channel_position_t> m_position_codes;
		std::vector<std::string> m_position_names;
		std::vector<std::array<double, 3>> m_positions;
		uint32_t m_sink_port_count, m_source_port_count;

		osd::audio_rate_range m_rate;

		node_info(sound_pulse *pulse, uint32_t id, uint32_t osdid, int type, std::string name, std::string desc) : m_pulse(pulse), m_id(id), m_osdid(osdid), m_type(type), m_name(name), m_desc(desc), m_sink_port_count(0), m_source_port_count(0), m_rate{0, 0, 0} {
		}
	};

	struct stream_info {
		sound_pulse *m_pulse;
		uint32_t m_osdid;
		uint32_t m_pulse_id;
		node_info *m_target_node;
		uint32_t m_channels;
		pa_stream *m_stream;
		std::vector<float> m_volumes;
		abuffer m_buffer;

		stream_info(sound_pulse *pulse, uint32_t osdid, uint32_t channels) : m_pulse(pulse), m_osdid(osdid), m_pulse_id(0), m_channels(channels), m_stream(nullptr), m_volumes(channels), m_buffer(channels) {}
	};

	std::map<uint32_t, node_info> m_nodes;
	std::map<uint32_t, uint32_t> m_node_osdid_to_id;

	std::map<uint32_t, stream_info> m_streams;
	std::map<uint32_t, uint32_t> m_stream_pulse_id_to_osdid;

	pa_threaded_mainloop *m_mainloop;
	pa_context *m_context;
	uint32_t m_node_current_id, m_stream_current_id;
	uint32_t m_generation;
	bool m_wait_stream, m_wait_init;

	std::string m_default_audio_sink;
	std::string m_default_audio_source;

	static void i_server_info(pa_context *, const pa_server_info *i, void *self);
	void server_info(const pa_server_info *i);
	static void i_context_notify(pa_context *, void *self);
	void context_notify();
	static void i_context_subscribe(pa_context *, pa_subscription_event_type_t t, uint32_t idx, void *self);
	void context_subscribe(pa_subscription_event_type_t t, uint32_t idx);
	static void i_stream_notify(pa_stream *, void *self);
	void stream_notify(stream_info *stream);
	static void i_stream_write_request(pa_stream *, size_t size, void *self);
	void stream_write_request(stream_info *stream, size_t size);
	static void i_source_info(pa_context *, const pa_source_info *i, int eol, void *self);
	void source_info(const pa_source_info *i, int eol);
	static void i_sink_info_new(pa_context *, const pa_sink_info *i, int eol, void *self);
	void sink_info_new(const pa_sink_info *i, int eol);
	static void i_sink_input_info_change(pa_context *, const pa_sink_input_info *i, int eol, void *self);
	void sink_input_info_change(stream_info *stream, const pa_sink_input_info *i, int eol);

	void generic_error(const char *msg);
	void generic_pa_error(const char *msg, int err);
};

// Try to more or less map to speaker.h positions

const sound_pulse::position_info sound_pulse::position_infos[] = {
	{ PA_CHANNEL_POSITION_MONO,         {  0.0,  0.0,  1.0 } },
	{ PA_CHANNEL_POSITION_FRONT_LEFT,   { -0.2,  0.0,  1.0 } },
	{ PA_CHANNEL_POSITION_FRONT_RIGHT,  {  0.2,  0.0,  1.0 } },
	{ PA_CHANNEL_POSITION_FRONT_CENTER, {  0.0,  0.0,  1.0 } },
	{ PA_CHANNEL_POSITION_LFE,          {  0.0, -0.5,  1.0 } },
	{ PA_CHANNEL_POSITION_REAR_LEFT,    { -0.2,  0.0, -0.5 } },
	{ PA_CHANNEL_POSITION_REAR_RIGHT,   {  0.2,  0.0, -0.5 } },
	{ PA_CHANNEL_POSITION_REAR_CENTER,  {  0.0,  0.0, -0.5 } },
	{ PA_CHANNEL_POSITION_MAX,          {  0.0,  0.0,  0.0 } }
};


const char *const sound_pulse::typenames[] = {
	"Audio recorder", "Speaker"
};

void sound_pulse::generic_error(const char *msg)
{
	perror(msg);
	::exit(1);
}

void sound_pulse::generic_pa_error(const char *msg, int err)
{
	fprintf(stderr, "%s: %s\n", msg, pa_strerror(err));
	::exit(1);
}

void sound_pulse::context_notify()
{
	fprintf(stderr, "context notify\n");
	pa_context_state state = pa_context_get_state(m_context);
	if(state == PA_CONTEXT_READY) {
		pa_context_subscribe(m_context, PA_SUBSCRIPTION_MASK_ALL, nullptr, this);
		pa_context_get_sink_info_list(m_context, i_sink_info_new, (void *)this);

	} else if(state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
		m_generation = 0x80000000;
		pa_threaded_mainloop_signal(m_mainloop, 0);
	}
}

void sound_pulse::i_context_notify(pa_context *, void *self)
{
	static_cast<sound_pulse *>(self)->context_notify();
}

void sound_pulse::stream_notify(stream_info *stream)
{
	pa_stream_state state = pa_stream_get_state(stream->m_stream);

	fprintf(stderr, "stream notify\n");
	if(state == PA_STREAM_READY || state == PA_STREAM_FAILED || state == PA_STREAM_TERMINATED) {
		m_wait_stream = false;
		pa_threaded_mainloop_signal(m_mainloop, 0);
	}
}

void sound_pulse::i_stream_notify(pa_stream *, void *self)
{
	stream_info *si = static_cast<stream_info *>(self);
	si->m_pulse->stream_notify(si);
}

void sound_pulse::stream_write_request(stream_info *stream, size_t size)
{
	// This is called with the thread locked
	while(size) {
		void *buffer;
		size_t bsize = size;
		int err = pa_stream_begin_write(stream->m_stream, &buffer, &bsize);
		if(err)
			generic_pa_error("stream begin write", err);
		uint32_t frames = bsize/2/stream->m_channels;
		uint32_t bytes = frames*2*stream->m_channels;
		stream->m_buffer.get((int16_t *)buffer, frames);
		err = pa_stream_write(stream->m_stream, buffer, bytes, nullptr, 0, PA_SEEK_RELATIVE);
		if(err)
			generic_pa_error("stream write", err);
		size -= bytes;
	}
}

void sound_pulse::i_stream_write_request(pa_stream *, size_t size, void *self)
{
	stream_info *si = static_cast<stream_info *>(self);
	si->m_pulse->stream_write_request(si, size);
}


void sound_pulse::server_info(const pa_server_info *i)
{
	m_default_audio_sink = i->default_sink_name;
	m_default_audio_source = i->default_source_name;
	fprintf(stderr, "defaults %s %s\n", i->default_sink_name, i->default_source_name);
	m_generation++;
	if(m_wait_init) {
		m_wait_init = false;
		pa_threaded_mainloop_signal(m_mainloop, 0);
	}
}

void sound_pulse::i_server_info(pa_context *, const pa_server_info *i, void *self)
{
	static_cast<sound_pulse *>(self)->server_info(i);
}

void sound_pulse::source_info(const pa_source_info *i, int eol)
{
	if(eol) {
		if(m_wait_init)
			pa_context_get_server_info(m_context, i_server_info, (void *)this);
		return;
	}
	auto ni = m_nodes.find(i->index);
	if(ni != m_nodes.end()) {
		// Add the monitoring sources to the node
		ni->second.m_source_port_count = i->channel_map.channels;
		return;
	}

	fprintf(stderr, "new source %d (%s)\n", i->index, i->description);
	m_node_osdid_to_id[m_node_current_id] = i->index;
	auto &node = m_nodes.emplace(i->index, node_info(this, i->index, m_node_current_id++, AREC, i->name, i->description)).first->second;

	node.m_source_port_count = i->channel_map.channels;
	for(int chan=0; chan != i->channel_map.channels; chan++) {
		pa_channel_position_t pos = i->channel_map.map[chan];
		node.m_position_codes.push_back(pos);
		node.m_position_names.push_back(pa_channel_position_to_pretty_string(pos));
		for(uint32_t j = 0;; j++) {
			if((position_infos[j].m_position == pos) || (position_infos[j].m_position == PA_CHANNEL_POSITION_MAX)) {
				node.m_positions.push_back(position_infos[j].m_coords);
				break;
			}
		}
	}
}

void sound_pulse::i_source_info(pa_context *, const pa_source_info *i, int eol, void *self)
{
	static_cast<sound_pulse *>(self)->source_info(i, eol);
}

void sound_pulse::sink_info_new(const pa_sink_info *i, int eol)
{
	if(eol) {
		if(m_wait_init)
			pa_context_get_source_info_list(m_context, i_source_info, (void *)this);
		return;
	}

	fprintf(stderr, "new sink %d (%s)\n", i->index, i->description);
	fprintf(stderr, "rate %d\n", i->sample_spec.rate);
	m_node_osdid_to_id[m_node_current_id] = i->index;
	auto &node = m_nodes.emplace(i->index, node_info(this, i->index, m_node_current_id++, APLAY, i->name, i->description)).first->second;

	node.m_sink_port_count = i->channel_map.channels;
	for(int chan=0; chan != i->channel_map.channels; chan++) {
		pa_channel_position_t pos = i->channel_map.map[chan];
		node.m_position_codes.push_back(pos);
		node.m_position_names.push_back(pa_channel_position_to_pretty_string(pos));
		for(uint32_t j = 0;; j++) {
			if((position_infos[j].m_position == pos) || (position_infos[j].m_position == PA_CHANNEL_POSITION_MAX)) {
				node.m_positions.push_back(position_infos[j].m_coords);
				break;
			}
		}
	}
	m_generation++;
}

void sound_pulse::i_sink_info_new(pa_context *, const pa_sink_info *i, int eol, void *self)
{
	static_cast<sound_pulse *>(self)->sink_info_new(i, eol);
}


void sound_pulse::sink_input_info_change(stream_info *stream, const pa_sink_input_info *i, int eol)
{
	if(eol)
		return;

	auto ni = m_nodes.find(i->sink);
	if(ni != m_nodes.end())
		stream->m_target_node = &ni->second;

	for(uint32_t port = 0; port != stream->m_channels; port++)
		stream->m_volumes[port] = pa_sw_volume_to_dB(i->volume.values[port]);

	fprintf(stderr, "change stream %d/%d sink=%s [%f %f]\n", stream->m_osdid, stream->m_pulse_id, stream->m_target_node->m_desc.c_str(), stream->m_volumes[0], stream->m_volumes[1]);
	m_generation++;
}

void sound_pulse::i_sink_input_info_change(pa_context *, const pa_sink_input_info *i, int eol, void *self)
{
	fprintf(stderr, "i_sink_input_info_change %p %d\n", i, eol);
	stream_info *stream = static_cast<stream_info *>(self);
	stream->m_pulse->sink_input_info_change(stream, i, eol);
}

void sound_pulse::context_subscribe(pa_subscription_event_type_t t, uint32_t idx)
{
	// This is called with the thread locked
	static const char *const evt[] = { "sink", "source", "sink-input", "source-output", "module", "client", "cache", "server", "autoload", "card" };
	static const char *const evt2[] = { "new", "change", "remove" };
	switch(int(t)) {
	case PA_SUBSCRIPTION_EVENT_REMOVE | PA_SUBSCRIPTION_EVENT_SINK:
	case PA_SUBSCRIPTION_EVENT_REMOVE | PA_SUBSCRIPTION_EVENT_SOURCE: {
		auto si = m_nodes.find(idx);
		if(si == m_nodes.end())
			break;
		fprintf(stderr, "removing %s\n", si->second.m_desc.c_str());
		for(auto &istream : m_streams)
			if(istream.second.m_target_node == &si->second)
				istream.second.m_target_node = nullptr;
		m_nodes.erase(si);
		m_generation++;
		break;
	}		

	case PA_SUBSCRIPTION_EVENT_NEW | PA_SUBSCRIPTION_EVENT_SINK:
		pa_context_get_sink_info_by_index(m_context, idx, i_sink_info_new, this);
		break;

	case PA_SUBSCRIPTION_EVENT_NEW | PA_SUBSCRIPTION_EVENT_SOURCE:
		pa_context_get_source_info_by_index(m_context, idx, i_source_info, this);
		break;

	case PA_SUBSCRIPTION_EVENT_CHANGE | PA_SUBSCRIPTION_EVENT_SERVER:
		pa_context_get_server_info(m_context, i_server_info, (void *)this);
		break;

	case PA_SUBSCRIPTION_EVENT_CHANGE | PA_SUBSCRIPTION_EVENT_SINK_INPUT: {
		auto si1 = m_stream_pulse_id_to_osdid.find(idx);
		if(si1 == m_stream_pulse_id_to_osdid.end())
			break;
		auto si = m_streams.find(si1->second);
		if(si == m_streams.end())
			break;
		pa_context_get_sink_input_info(m_context, idx, i_sink_input_info_change, (void *)&si->second);
		break;
	}

	default:
		fprintf(stderr, "event %s %s  %d\n", evt2[t>>4], evt[t&15], idx);
	}
}

void sound_pulse::i_context_subscribe(pa_context *, pa_subscription_event_type_t t, uint32_t idx, void *self)
{
	static_cast<sound_pulse *>(self)->context_subscribe(t, idx);
}



int sound_pulse::init(osd_interface &osd, osd_options const &options)
{
	m_node_current_id = 1;
	m_stream_current_id = 1;
	m_generation = 0;
	m_wait_stream = false;
	m_wait_init = true;

	m_mainloop = pa_threaded_mainloop_new();
	m_context = pa_context_new(pa_threaded_mainloop_get_api(m_mainloop), "MAME");
	pa_context_set_state_callback(m_context, i_context_notify, this);
	pa_context_set_subscribe_callback(m_context, i_context_subscribe, this);
	pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
	pa_threaded_mainloop_start(m_mainloop);

	pa_threaded_mainloop_lock(m_mainloop);
	while(m_wait_init)
		pa_threaded_mainloop_wait(m_mainloop);
	pa_threaded_mainloop_unlock(m_mainloop);
	if(m_generation >= 0x80000000)
		return 1;

	return 0;
}

uint32_t sound_pulse::get_generation()
{
	pa_threaded_mainloop_lock(m_mainloop);
	uint32_t result = m_generation;
	pa_threaded_mainloop_unlock(m_mainloop);
	return result;
}

osd::audio_info sound_pulse::get_information()
{
	osd::audio_info result;
	pa_threaded_mainloop_lock(m_mainloop);
	result.m_nodes.resize(m_nodes.size());
	result.m_default_sink = 0;
	result.m_default_source = 0;
	result.m_generation = m_generation;
	uint32_t node = 0;
	for(auto &inode : m_nodes) {
		result.m_nodes[node].m_name = inode.second.m_desc;
		result.m_nodes[node].m_id = inode.second.m_osdid;
		result.m_nodes[node].m_rate = inode.second.m_rate;
		result.m_nodes[node].m_sinks = inode.second.m_sink_port_count;
		result.m_nodes[node].m_sources = inode.second.m_source_port_count;
		result.m_nodes[node].m_port_names = inode.second.m_position_names;
		result.m_nodes[node].m_port_positions = inode.second.m_positions;

		if(inode.second.m_name == m_default_audio_sink)
			result.m_default_sink = inode.second.m_osdid;
		if(inode.second.m_name == m_default_audio_source)
			result.m_default_source = inode.second.m_osdid;
		node ++;
	}

	for(auto &istream : m_streams)
		if(istream.second.m_target_node)
			result.m_streams.emplace_back(osd::audio_info::stream_info { istream.second.m_osdid, istream.second.m_target_node->m_osdid, istream.second.m_volumes });

	pa_threaded_mainloop_unlock(m_mainloop);
	return result;
}

uint32_t sound_pulse::stream_sink_open(uint32_t node, std::string name, uint32_t rate)
{
	pa_threaded_mainloop_lock(m_mainloop);
	auto ni = m_node_osdid_to_id.find(node);
	if(ni == m_node_osdid_to_id.end()) {
		pa_threaded_mainloop_unlock(m_mainloop);
		return 0;
	}
	node_info &snode = m_nodes.find(ni->second)->second;

	uint32_t id = m_stream_current_id++;
	auto &stream = m_streams.emplace(id, stream_info(this, id, snode.m_sink_port_count)).first->second;

	pa_sample_spec ss;
#ifdef LSB_FIRST
	ss.format = PA_SAMPLE_S16LE;
#else
	ss.format = PA_SAMPLE_S16BE;
#endif
	ss.rate = rate;
	ss.channels = stream.m_channels;
	stream.m_stream = pa_stream_new(m_context, name.c_str(), &ss, nullptr);
	pa_stream_set_state_callback(stream.m_stream, i_stream_notify, &stream);
	pa_stream_set_write_callback(stream.m_stream, i_stream_write_request, &stream);

	pa_buffer_attr battr;
	battr.fragsize = uint32_t(-1);
	battr.maxlength = 1024;
	battr.minreq = uint32_t(-1);
	battr.prebuf = uint32_t(-1);
	battr.tlength = uint32_t(-1);

	int err = pa_stream_connect_playback(stream.m_stream, snode.m_name.c_str(), &battr, pa_stream_flags_t(PA_STREAM_ADJUST_LATENCY|PA_STREAM_START_UNMUTED), nullptr, nullptr);
	if(err)
		generic_pa_error("stream connect playback", err);

	stream.m_target_node = &snode;

	m_wait_stream = true;
	while(m_wait_stream)
		pa_threaded_mainloop_wait(m_mainloop);

	stream.m_pulse_id = pa_stream_get_index(stream.m_stream);
	m_stream_pulse_id_to_osdid[stream.m_pulse_id] = id;

	fprintf(stderr, "stream id %d\n", stream.m_pulse_id);
	pa_threaded_mainloop_unlock(m_mainloop);

	return id;
}

void sound_pulse::stream_sink_update(uint32_t id, const int16_t *buffer, int samples_this_frame)
{
	pa_threaded_mainloop_lock(m_mainloop);
	auto si = m_streams.find(id);
	if(si == m_streams.end()) {
		pa_threaded_mainloop_unlock(m_mainloop);
		return;
	}
	si->second.m_buffer.push(buffer, samples_this_frame);
	pa_threaded_mainloop_unlock(m_mainloop);
}

void sound_pulse::stream_set_volumes(uint32_t id, const std::vector<float> &db)
{
}

void sound_pulse::stream_close(uint32_t id)
{
	pa_threaded_mainloop_lock(m_mainloop);
	auto si = m_streams.find(id);
	if(si == m_streams.end()) {
		pa_threaded_mainloop_unlock(m_mainloop);
		return;
	}
	stream_info &stream = si->second;
	pa_stream_set_state_callback(stream.m_stream, nullptr, &stream);
	pa_stream_set_write_callback(stream.m_stream, nullptr, &stream);
	pa_stream_disconnect(stream.m_stream);
	m_streams.erase(si);
	pa_threaded_mainloop_unlock(m_mainloop);
}

void sound_pulse::exit()
{
	for(const auto &si : m_streams) {
		pa_stream_disconnect(si.second.m_stream);
		pa_stream_unref(si.second.m_stream);
	}

	pa_context_unref(m_context);
	pa_threaded_mainloop_free(m_mainloop);
}

#else
	MODULE_NOT_SUPPORTED(sound_pulse, OSD_SOUND_PROVIDER, "pulse")
#endif

MODULE_DEFINITION(SOUND_PULSEAUDIO, sound_pulse)
