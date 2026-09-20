// license:BSD-3-Clause
// copyright-holders:R. Belmont, Vas Crabb
//============================================================
//
//  coreaudio_sound.cpp - CoreAudio backend for MAME
//
//============================================================

#include "sound_module.h"
#include "modules/osdmodule.h"

#ifdef SDLMAME_MACOSX

#include "modules/lib/osdobj_common.h"

#include <AvailabilityMacros.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <iterator>
#include <map>
#include <memory>
#include <new>
#include <cstring>

#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 120000
#define PROPERTY_ELEMENT_MASTER kAudioObjectPropertyElementMain
#else
#define PROPERTY_ELEMENT_MASTER kAudioObjectPropertyElementMaster
#endif

namespace osd
{
namespace
{
static const char *const sMacChannelLabels[] =
{
	"",
	"Front Left",                   // 1
	"Front Right",
	"Front Center",
	"Low Frequency Effects",
	"Rear Left",
	"Rear Right",
	"Front Left of Center",
	"Front Right of Center",
	"Rear Center",
	"Side Left",                    // 10
	"Side Right",
	"Top Center",
	"Top Front Left",
	"Top Front Center",
	"Top Front Right",
	"Top Rear Left",
	"Top Rear Center",
	"Top Rear Right",
	"", "",                         // 20
	"", "", "", "", "", "", "", "",
	"", "",                         // 30
	"", "",
	"Rear Surround Left",
	"Rear Surround Right",
	"Left Wide",
	"Right Wide",
	"Low Frequency Effects 2",
	"Left Total",
	"Right Total",
	"Hearing Impaired",             // 40
	"Narration",
	"Mono",
	"Dialog Centric Mix",
	"Center Surround Direct",
	"Haptic",
	"", "", "",
	"Left Top Middle",
	"",                            // 50 (no label is defined for 50)
	"Right Top Middle",
	"Left Top Rear",
	"Center Top Rear",
	"Right Top Rear",
	"Left Side Surround",
	"Right Side Surround",
	"Left Bottom",
	"Right Bottom",
	"Center Bottom",
	"Left Top Surround",            // 60
	"Right Top Surround",
	"Low Frequency Effects 3",
	"Left Back Surround",
	"Right Back Surround",
	"Left Edge of Screen",
	"Right Edge of Screen"          // 66
};

// Channel positions in MAME 3D space.
// The listener is at (0, 0, 0), with positive X to the right, positive Y up, and positive Z in front.
// Copying the core's convention, left is X = -0.2, center is X = 0.0, and right is X = 0.2.
// Front Z is 1.0, back Z is -0.5, top Y is 0.5, and bottom Y is -0.5.
// "Surround" channels are at X = -0.4 (left) and 0.4 (right).
static const osd::channel_position sChannelPositions[] =
{
	osd::channel_position::UNKNOWN(), // unused
	osd::channel_position::FL(), // Front Left
	osd::channel_position::FR(), // Front Right
	osd::channel_position::FC(), // Front Center
	osd::channel_position::LFE(), // Low Frequency Effects
	osd::channel_position::RL(), // Rear Left
	osd::channel_position::RR(), // Rear Right
	osd::channel_position( -0.1,  0.0,  1.0 ), // Front Left of Center
	osd::channel_position(  0.1,  0.0,  1.0 ), // Front Right of Center
	osd::channel_position::RC(), // Rear Center
	osd::channel_position( -0.2,  0.0,  0.5 ), // Side Left
	osd::channel_position(  0.2,  0.0,  0.5 ), // Side Right
	osd::channel_position(  0.0,  0.5, -0.1 ), // Top Center
	osd::channel_position( -0.2,  0.5,  1.0 ), // Top Front Left
	osd::channel_position(  0.0,  0.5,  1.0 ), // Top Front Center
	osd::channel_position(  0.2,  0.5,  1.0 ), // Top Front Right
	osd::channel_position( -0.2,  0.5, -0.5 ), // Top Rear Left
	osd::channel_position(  0.0,  0.5, -0.5 ), // Top Rear Center
	osd::channel_position(  0.2,  0.5, -0.5 ), // Top Rear Right
	osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(),
	osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(),
	osd::channel_position::UNKNOWN(), osd::channel_position::UNKNOWN(),
	osd::channel_position( -0.4,  0.0, -0.5 ), // Rear Surround Left
	osd::channel_position(  0.4,  0.0, -0.5 ), // Rear Surround Right
	osd::channel_position( -0.4,  0.0,  1.0 ), // Left Wide
	osd::channel_position(  0.4,  0.0,  1.0 ), // Right Wide
	osd::channel_position::LFE(), // Low Frequency Effects 2
	osd::channel_position::HL(), // Left Total
	osd::channel_position::HR(), // Right Total
	osd::channel_position::FC(), // Hearing Impaired
	osd::channel_position::FC(), // Narration
	osd::channel_position::FC(), // Mono
	osd::channel_position::FC(), // Dialog Centric Mix
	osd::channel_position::HC(), // Center Surround Direct
	osd::channel_position::UNKNOWN(), // Haptic
	osd::channel_position::UNKNOWN(), // unused
	osd::channel_position::UNKNOWN(), // unused
	osd::channel_position::UNKNOWN(), // unused
	osd::channel_position( -0.2,  0.5,  0.0 ), // Left Top Middle
	osd::channel_position::UNKNOWN(), // unused
	osd::channel_position(  0.2,  0.5,  0.0 ), // Right Top Middle
	osd::channel_position( -0.2,  0.5, -0.5 ), // Left Top Rear
	osd::channel_position(  0.0,  0.5, -0.5 ), // Center Top Rear
	osd::channel_position(  0.2,  0.5, -0.5 ), // Right Top Rear
	osd::channel_position( -0.4,  0.0, -0.1 ), // Left Side Surround
	osd::channel_position(  0.4,  0.0, -0.1 ), // Right Side Surround
	osd::channel_position( -0.2, -0.5,  0.0 ), // Left Bottom
	osd::channel_position(  0.2, -0.5,  0.0 ), // Right Bottom
	osd::channel_position(  0.0, -0.5,  0.0 ), // Center Bottom
	osd::channel_position( -0.4,  0.5, -0.1 ), // Left Top Surround
	osd::channel_position(  0.4,  0.5, -0.1 ), // Right Top Surround
	osd::channel_position::LFE(), // Low Frequency Effects 3
	osd::channel_position( -0.4,  0.0, -0.5 ), // Left Rear Surround
	osd::channel_position(  0.4,  0.0, -0.5 ), // Right Rear Surround
	osd::channel_position( -0.1,  0.0,  1.0 ), // Left Edge of Screen
	osd::channel_position(  0.1,  0.0,  1.0 )  // Right Edge of Screen
};

// Both tables are indexed by kAudioChannelLabel_* values, so they have to stay in step -
// a missing comma in either one silently shifts every entry after it.
static_assert(
		std::size(sMacChannelLabels) == std::size(sChannelPositions),
		"channel label and position tables must have the same number of entries");
static constexpr int sMacChannelCount = int(std::size(sMacChannelLabels));

struct coreaudio_device
{
	std::string m_name;
	std::string m_uid;
	AudioDeviceID m_id;
	// m_sinks/m_sources are stream counts, m_sink_channels/m_source_channels are channel
	// counts - a device commonly presents several channels on a single stream, so the two
	// must not be conflated
	int m_sinks;
	int m_sources;
	int m_sink_channels;
	int m_source_channels;
	int m_sample_rate;

	coreaudio_device(const char *name, const char *uid, AudioDeviceID id, int sinks, int sources, int sink_channels, int source_channels, int sample_rate) :
		m_name(name),
		m_uid(uid),
		m_id(id),
		m_sinks(sinks),
		m_sources(sources),
		m_sink_channels(sink_channels),
		m_source_channels(source_channels),
		m_sample_rate(sample_rate)
	{
	}
};

class sound_coreaudio : public osd_module, public sound_module
{
public:
	sound_coreaudio() :
		osd_module(OSD_SOUND_PROVIDER, "coreaudio"),
		sound_module(),
		m_sample_rate(0),
		m_audio_latency(0.0f)
	{
	}

	virtual ~sound_coreaudio()
	{
	}

	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override;

	// sound_module
	virtual uint32_t get_generation() override;
	virtual osd::audio_info get_information() override;
	virtual uint32_t stream_sink_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual uint32_t stream_source_open(uint32_t node, std::string name, uint32_t rate) override;
	virtual void stream_source_update(uint32_t id, int16_t *buffer, int samples_this_frame) override;
	virtual void stream_set_volumes(uint32_t id, const std::vector<float> &db) override;
	virtual void stream_close(uint32_t id) override;
	virtual void stream_sink_update(uint32_t stream_id, int16_t const *buffer, int samples_this_frame) override;

	virtual bool external_per_channel_volume() override { return false; }
	virtual bool split_streams_per_source() override { return true; }

private:
	static std::string selector_name(AudioObjectPropertySelector selector);
	bool set_property_listener(AudioDeviceID device, AudioObjectPropertySelector selector, AudioObjectPropertyScope scope);
	bool clear_property_listener(AudioDeviceID device, AudioObjectPropertySelector selector, AudioObjectPropertyScope scope);
	void set_device_listeners();
	void clear_device_listeners();

	OSStatus property_changed(
		AudioObjectID inObjectID,
		UInt32 inNumberAddresses,
		const AudioObjectPropertyAddress inAddresses[])
	{
		for (UInt32 i = 0; i < inNumberAddresses; i++)
		{
			osd_printf_verbose("CoreAudio: property %s changed\n", selector_name(inAddresses[i].mSelector).c_str());

			m_need_generation_bump = true;
		}

		return noErr;
	}

	static OSStatus property_callback(
		AudioObjectID inObjectID,
		UInt32 inNumberAddresses,
		const AudioObjectPropertyAddress inAddresses[],
		void *inClientData)
	{
		return ((sound_coreaudio *)inClientData)->property_changed(inObjectID, inNumberAddresses, inAddresses);
	}

	class coreaudio_stream
	{
	public:
		coreaudio_stream(int input_channels, uint32_t rate) :
			m_input_buffer(input_channels, rate),
			m_graph(nullptr),
			m_is_source(false),
			m_node_count(0),
			m_channels(input_channels),
			m_sample_rate(0),
			m_audio_latency(0.0f),
			m_sample_bytes(0),
			m_headroom(0),
			m_buffer_size(0),
			m_max_frames(0),
			m_buffer(),
			m_playpos(0),
			m_writepos(0),
			m_in_underrun(false),
			m_overflows(0),
			m_underflows(0),
			m_render_errors(0)
		{
		}

		int get_device() { return m_id; }
		int create_sink_stream(struct coreaudio_device &device, const char *name, int sample_rate, float latency);
		int create_source_stream(struct coreaudio_device &device, const char *name, int sample_rate, float latency);
		void sink_update(int16_t const *buffer, int samples_this_frame);
		void close();

		std::mutex m_stream_mutex;
		sound_module::abuffer m_input_buffer;

	private:
		struct node_detail
		{
			AUNode m_node = 0;
			AudioUnit m_unit = nullptr;
		};

		enum
		{
			EFFECT_COUNT_MAX = 10
		};

		uint32_t buffer_avail() const { return ((m_writepos >= m_playpos) ? m_buffer_size : 0) + m_playpos - m_writepos; }
		uint32_t buffer_used() const { return ((m_playpos > m_writepos) ? m_buffer_size : 0) + m_writepos - m_playpos; }

		bool create_sink_graph(struct coreaudio_device &device);
		bool create_source_graph(struct coreaudio_device &device);
		bool add_device_output(struct coreaudio_device &device);
		bool add_device_input(struct coreaudio_device &device);
		bool add_converter();

		OSStatus add_node(OSType type, OSType subtype, OSType manufacturer)
		{
			AudioComponentDescription const desc = { type, subtype, manufacturer, 0, 0 };
			return AUGraphAddNode(m_graph, &desc, &m_node_details[m_node_count].m_node);
		}

		OSStatus get_next_node_info()
		{
			return AUGraphNodeInfo(
				m_graph,
				m_node_details[m_node_count].m_node,
				nullptr,
				&m_node_details[m_node_count].m_unit);
		}

		OSStatus connect_next_node()
		{
			return AUGraphConnectNodeInput(
				m_graph,
				m_node_details[m_node_count].m_node,
				0,
				m_node_details[m_node_count - 1].m_node,
				0);
		}

		OSStatus sink_render(
			AudioUnitRenderActionFlags *action_flags,
			const AudioTimeStamp *timestamp,
			UInt32 bus_number,
			UInt32 number_frames,
			AudioBufferList *data);

		static OSStatus sink_render_callback(
			void *refcon,
			AudioUnitRenderActionFlags *action_flags,
			const AudioTimeStamp *timestamp,
			UInt32 bus_number,
			UInt32 number_frames,
			AudioBufferList *data);

		OSStatus source_render(
			AudioUnitRenderActionFlags *action_flags,
			const AudioTimeStamp *timestamp,
			UInt32 bus_number,
			UInt32 number_frames,
			AudioBufferList *data);

		static OSStatus source_render_callback(
			void *refcon,
			AudioUnitRenderActionFlags *action_flags,
			const AudioTimeStamp *timestamp,
			UInt32 bus_number,
			UInt32 number_frames,
			AudioBufferList *data);

		AudioDeviceID m_id;
		AUGraph m_graph;
		bool m_is_source;
		unsigned m_node_count;
		node_detail m_node_details[EFFECT_COUNT_MAX + 2];

		int32_t m_channels;
		int32_t m_sample_rate;
		float m_audio_latency;
		uint32_t m_sample_bytes;
		uint32_t m_headroom;
		uint32_t m_buffer_size;
		uint32_t m_max_frames;
		std::unique_ptr<int8_t[]> m_buffer;
		uint32_t m_playpos;
		uint32_t m_writepos;
		bool m_in_underrun;
		unsigned m_overflows;
		unsigned m_underflows;
		std::atomic<unsigned> m_render_errors;
	};
	struct coreaudio_stream_info
	{
		std::string m_name;
		AudioDeviceID m_id;
		std::shared_ptr<coreaudio_stream> m_stream;
		std::vector<float> m_volumes;

		coreaudio_stream_info(int channels, uint32_t rate)
		{
			m_stream = std::make_shared<coreaudio_stream>(channels, rate);
		}
	};

	void rebuild_stream_info();
	bool get_output_device_id(char const *name, AudioDeviceID &id) const;
	AudioDeviceID get_default_sink();
	AudioDeviceID get_default_source();
	std::unique_ptr<char[]> get_device_uid(AudioDeviceID id) const;
	std::unique_ptr<char[]> get_device_name(AudioDeviceID id) const;
	void build_device_list(void);
	UInt32 get_input_stream_count(
		AudioDeviceID id,
		char const *uid,
		char const *name) const;
	UInt32 get_output_stream_count(
		AudioDeviceID id,
		char const *uid,
		char const *name) const;
	UInt32 get_channel_count(
		AudioDeviceID id,
		AudioObjectPropertyScope scope,
		char const *name) const;

	std::unique_ptr<char[]> convert_cfstring_to_utf8(CFStringRef str) const
	{
		CFIndex const len = CFStringGetMaximumSizeForEncoding(
			CFStringGetLength(str),
			kCFStringEncodingUTF8);
		std::unique_ptr<char[]> result = std::make_unique<char[]>(len + 1);
		if (!CFStringGetCString(str, result.get(), len + 1, kCFStringEncodingUTF8))
			result.reset();
		return result;
	}

	int m_sample_rate;
	float m_audio_latency;
	osd::audio_info m_deviceinfo;
	std::atomic<uint32_t> m_stream_id = 1;   // zero means "no stream" to the core, so never hand it out
	std::map<AudioDeviceID, coreaudio_device> m_device_list;
	std::map<uint32_t, coreaudio_stream_info> m_stream_list;
	std::mutex m_stream_list_mutex;
	std::atomic<bool> m_need_generation_bump;
};

int sound_coreaudio::init(osd_interface &osd, const osd_options &options)
{
	m_sample_rate = options.sample_rate();
	m_audio_latency = options.audio_latency();

	// build the list of available CoreAudio devices
	build_device_list();
	m_need_generation_bump = false;

	// set up notifications
	if (!set_property_listener(kAudioObjectSystemObject, kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal))
		return -1;
	if (!set_property_listener(kAudioObjectSystemObject, kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal))
		return -1;
	if (!set_property_listener(kAudioObjectSystemObject, kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal))
		return -1;

	m_deviceinfo.m_generation = 1;
	m_deviceinfo.m_default_sink = get_default_sink();
	m_deviceinfo.m_default_source = get_default_source();

	set_device_listeners();

	// Show the defaults
	auto default_sink = m_device_list.find(get_default_sink());
	if (default_sink != m_device_list.end())
	{
		osd_printf_verbose("CoreAudio: default output device is %s (%s)\n", default_sink->second.m_name, default_sink->second.m_uid);
	}
	auto default_source = m_device_list.find(get_default_source());
	if (default_source != m_device_list.end())
	{
		osd_printf_verbose("CoreAudio: default input device is %s (%s)\n", default_source->second.m_name, default_source->second.m_uid);
	}

	osd_printf_verbose("CoreAudio: End initialization\n");
	return 0;
}

void sound_coreaudio::exit()
{
	osd_printf_verbose("CoreAudio: Shutting down\n");

	clear_property_listener(kAudioObjectSystemObject, kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal);
	clear_property_listener(kAudioObjectSystemObject, kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal);
	clear_property_listener(kAudioObjectSystemObject, kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal);

	std::lock_guard<std::mutex> list_guard(m_stream_list_mutex);
	clear_device_listeners();
	for (const auto &[key, stream] : m_stream_list)
	{
		stream.m_stream->close();
	}

	osd_printf_verbose("CoreAudio: Shutdown complete\n");
}

// Must be called with m_stream_list_mutex held, or otherwise safe
void sound_coreaudio::rebuild_stream_info()
{
	m_deviceinfo.m_streams.clear();
	for (const auto &[key, stream] : m_stream_list)
	{
		m_deviceinfo.m_streams.emplace_back(osd::audio_info::stream_info{ key, stream.m_id, stream.m_volumes });
	}
}

uint32_t sound_coreaudio::get_generation()
{
	if (m_need_generation_bump.exchange(false))
	{
		clear_device_listeners();

		std::lock_guard<std::mutex> list_guard(m_stream_list_mutex);
		m_deviceinfo.m_default_sink = get_default_sink();
		m_deviceinfo.m_default_source = get_default_source();
		build_device_list();
		m_deviceinfo.m_generation++;

		set_device_listeners();
	}

	return m_deviceinfo.m_generation;
}

osd::audio_info sound_coreaudio::get_information()
{
	return m_deviceinfo;
}

uint32_t sound_coreaudio::stream_sink_open(uint32_t node, std::string name, uint32_t rate)
{
	auto our_device = m_device_list.find(node);
	if (our_device != m_device_list.end())
	{
		if (our_device->second.m_sinks > 0)
		{
			struct coreaudio_stream_info stream(1, rate);

			if (!stream.m_stream->create_sink_stream(our_device->second, name.c_str(), rate, m_audio_latency))
			{
				stream.m_id = our_device->second.m_id;
				stream.m_name = name;
				std::lock_guard<std::mutex> list_guard(m_stream_list_mutex);
				const uint32_t new_id = m_stream_id++;
				m_stream_list.emplace(new_id, stream);

				rebuild_stream_info();

				osd_printf_verbose("CoreAudio: Created sink stream %d for device %d\n", new_id, our_device->second.m_id);
				return new_id;
			}
			osd_printf_error("CoreAudio: Failed to create stream for sink %d\n", node);
			return 0;
		}
	}

	osd_printf_error("CoreAudio: Failed to create stream for unknown sink %d\n", node);
	return 0;
}

uint32_t sound_coreaudio::stream_source_open(uint32_t node, std::string name, uint32_t rate)
{
	auto our_device = m_device_list.find(node);
	if (our_device != m_device_list.end())
	{
		// the stream carries one sample per channel, not per stream
		const auto channels = our_device->second.m_source_channels;

		if (channels > 0)
		{
			struct coreaudio_stream_info stream(channels, rate);

			if (!stream.m_stream->create_source_stream(our_device->second, name.c_str(), rate, m_audio_latency))
			{
				stream.m_id = our_device->second.m_id;
				stream.m_name = name;
				std::lock_guard<std::mutex> list_guard(m_stream_list_mutex);
				const uint32_t new_id = m_stream_id++;
				m_stream_list.emplace(new_id, stream);

				rebuild_stream_info();

				osd_printf_verbose("CoreAudio: Created source stream %d for device %d\n", new_id, our_device->second.m_id);
				return new_id;
			}
			osd_printf_error("CoreAudio: Failed to create stream for source %d\n", node);
			return 0;
		}
	}

	osd_printf_error("CoreAudio: Failed to create stream for unknown source %d\n", node);
	return 0;
}

void sound_coreaudio::stream_source_update(uint32_t node, int16_t *buffer, int samples_this_frame)
{
	std::lock_guard<std::mutex> list_guard(m_stream_list_mutex);

	auto our_stream = m_stream_list.find(node);
	if (our_stream != m_stream_list.end())
	{
		std::lock_guard<std::mutex> stream_guard(our_stream->second.m_stream->m_stream_mutex);
		our_stream->second.m_stream->m_input_buffer.get(buffer, samples_this_frame);
	}
}

void sound_coreaudio::stream_sink_update(uint32_t stream_id, int16_t const *buffer, int samples_this_frame)
{
	std::lock_guard<std::mutex> list_guard(m_stream_list_mutex);

	auto our_stream = m_stream_list.find(stream_id);
	if (our_stream != m_stream_list.end())
	{
		std::lock_guard<std::mutex> stream_guard(our_stream->second.m_stream->m_stream_mutex);
		our_stream->second.m_stream->sink_update(buffer, samples_this_frame);
	}
}

void sound_coreaudio::stream_set_volumes(uint32_t id, const std::vector<float> &db)
{
	std::lock_guard<std::mutex> list_guard(m_stream_list_mutex);

	auto our_stream = m_stream_list.find(id);
	if (our_stream != m_stream_list.end())
	{
		our_stream->second.m_volumes.clear();
		our_stream->second.m_volumes.reserve(db.size());
		for (const auto &volume : db)
		{
			our_stream->second.m_volumes.push_back(volume);
		}
	}
}

void sound_coreaudio::stream_close(uint32_t id)
{
	std::lock_guard<std::mutex> list_guard(m_stream_list_mutex);

	auto our_stream = m_stream_list.find(id);
	if (our_stream != m_stream_list.end())
	{
		osd_printf_verbose("CoreAudio: Closing stream %d on device %d\n", id, our_stream->second.m_stream->get_device());
		our_stream->second.m_stream->close();
		m_stream_list.erase(our_stream);
		rebuild_stream_info();
	}
}

// show property selectors by FourCC for better readability
std::string sound_coreaudio::selector_name(AudioObjectPropertySelector selector)
{
	char buf[4];
	for (int i = 0; i < 4; i++)
		buf[i] = char((selector >> (24 - (i * 8))) & 0xff);

	for (char const ch : buf)
	{
		if ((ch < 0x20) || (0x7e < ch))
		{
			char hex[16];
			snprintf(hex, std::size(hex), "0x%08x", unsigned(selector));
			return std::string(hex);
		}
	}

	return std::string("'") + std::string(buf, std::size(buf)) + "'";
}

bool sound_coreaudio::set_property_listener(AudioDeviceID device, AudioObjectPropertySelector selector, AudioObjectPropertyScope scope)
{
	AudioObjectPropertyAddress const property_addr = {
		selector,
		scope,
		PROPERTY_ELEMENT_MASTER };

	OSStatus err = AudioObjectAddPropertyListener(
			device,
			&property_addr,
			this->property_callback,
			this);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Could not set device %d %s listener (%d)\n",
				device,
				selector_name(selector),
				err);
		return false;
	}

	return true;
}

bool sound_coreaudio::clear_property_listener(AudioDeviceID device, AudioObjectPropertySelector selector, AudioObjectPropertyScope scope)
{
	AudioObjectPropertyAddress const property_addr = {
		selector,
		scope,
		PROPERTY_ELEMENT_MASTER };

	OSStatus err = AudioObjectRemovePropertyListener(
		device,
		&property_addr,
		this->property_callback,
		this);
	if (noErr != err)
	{
		// The device can be gone by the time we get here.  Its listeners were auto-destroyed
		// so there is nothing more to do.
		if ((kAudioHardwareBadObjectError == err) || (kAudioHardwareBadDeviceError == err))
		{
			osd_printf_verbose(
					"CoreAudio: Device %d went away before its %s listener could be removed\n",
					device,
					selector_name(selector));
			return true;
		}

		osd_printf_error(
				"CoreAudio: Could not remove device %d %s listener (%d)\n",
				device,
				selector_name(selector),
				err);
		return false;
	}

	return true;
}

void sound_coreaudio::set_device_listeners()
{
	for (const auto &[key, device] : m_device_list)
	{
		const AudioObjectPropertyScope scope = (device.m_sinks > 0) ? kAudioDevicePropertyScopeOutput : kAudioDevicePropertyScopeInput;

		set_property_listener(device.m_id, kAudioDevicePropertyPreferredChannelLayout, scope);
		set_property_listener(device.m_id, kAudioDevicePropertyStreamConfiguration, scope);
		set_property_listener(device.m_id, kAudioDevicePropertyNominalSampleRate, scope);
	}
}

void sound_coreaudio::clear_device_listeners()
{
	for (const auto &[key, device] : m_device_list)
	{
		const AudioObjectPropertyScope scope = (device.m_sinks > 0) ? kAudioDevicePropertyScopeOutput : kAudioDevicePropertyScopeInput;

		clear_property_listener(device.m_id, kAudioDevicePropertyPreferredChannelLayout, scope);
		clear_property_listener(device.m_id, kAudioDevicePropertyStreamConfiguration, scope);
		clear_property_listener(device.m_id, kAudioDevicePropertyNominalSampleRate, scope);
	}
}

bool sound_coreaudio::get_output_device_id(
	char const *name,
	AudioDeviceID &id) const
{
	// walk the device map
	for (const auto &[key, device] : m_device_list)
	{
		// we're only interested in output devices
		if (device.m_sinks > 0)
		{
			// if either the name or the UID matches, we'll take it
			bool matched = false;
			if (!strcmp(name, device.m_name.c_str()))
			{
				matched = true;
			}
			else if (!strcmp(name, device.m_uid.c_str()))
			{
				matched = true;
			}

			if (matched)
			{
				osd_printf_verbose(
						"CoreAudio: Matched device %s (%s) with %u output stream(s)\n",
						device.m_name,
						device.m_uid,
						device.m_sinks);

				id = key;
				return true;
			}
		}
	}

	osd_printf_verbose("CoreAudio: No audio output devices match %s\n", name);
	return false;
}

std::unique_ptr<char[]> sound_coreaudio::get_device_uid(AudioDeviceID id) const
{
	AudioObjectPropertyAddress const uid_addr = {
		kAudioDevicePropertyDeviceUID,
		kAudioObjectPropertyScopeGlobal,
		PROPERTY_ELEMENT_MASTER };
	CFStringRef device_uid = nullptr;
	UInt32 property_size = sizeof(device_uid);
	OSStatus const err = AudioObjectGetPropertyData(
			id,
			&uid_addr,
			0,
			nullptr,
			&property_size,
			&device_uid);
	if ((noErr != err) || (nullptr == device_uid))
	{
		osd_printf_warning(
				"CoreAudio: Error getting UID for audio device %u (%d)\n",
				id,
				err);
		return nullptr;
	}
	std::unique_ptr<char[]> result = convert_cfstring_to_utf8(device_uid);
	CFRelease(device_uid);
	if (!result)
	{
		osd_printf_warning(
				"CoreAudio: Error converting UID for audio device %u to UTF-8\n",
				id);
	}
	return result;
}

std::unique_ptr<char[]> sound_coreaudio::get_device_name(AudioDeviceID id) const
{
	AudioObjectPropertyAddress const name_addr = {
		kAudioDevicePropertyDeviceNameCFString,
		kAudioObjectPropertyScopeGlobal,
		PROPERTY_ELEMENT_MASTER };
	CFStringRef device_name = nullptr;
	UInt32 property_size = sizeof(device_name);
	OSStatus const err = AudioObjectGetPropertyData(
		id,
		&name_addr,
		0,
		nullptr,
		&property_size,
		&device_name);
	if ((noErr != err) || (nullptr == device_name))
	{
		osd_printf_warning(
				"CoreAudio: Error getting name for audio device %u (%d)\n",
				id,
				err);
		return nullptr;
	}
	std::unique_ptr<char[]> result = convert_cfstring_to_utf8(device_name);
	CFRelease(device_name);
	if (!result)
	{
		osd_printf_warning(
				"CoreAudio: Error converting name for audio device %u to UTF-8\n",
				id);
	}
	return result;
}

AudioDeviceID sound_coreaudio::get_default_sink()
{
	AudioDeviceID device_id;
	UInt32 dev_property_size = sizeof(device_id);
	AudioObjectPropertyAddress const def_id_address = {
		kAudioHardwarePropertyDefaultOutputDevice,
		kAudioObjectPropertyScopeGlobal,
		PROPERTY_ELEMENT_MASTER };

	OSStatus err = AudioObjectGetPropertyData(
			kAudioObjectSystemObject,
			&def_id_address, 0, NULL,
			&dev_property_size, &device_id);

	if (err != kAudioHardwareNoError)
	{
		osd_printf_error("CoreAudio: Error getting the default audio device.\n");
		return 0;
	}

	return device_id;
}

AudioDeviceID sound_coreaudio::get_default_source()
{
	AudioDeviceID device_id;
	UInt32 dev_property_size = sizeof(device_id);
	AudioObjectPropertyAddress const def_id_address = {
		kAudioHardwarePropertyDefaultInputDevice,
		kAudioObjectPropertyScopeGlobal,
		PROPERTY_ELEMENT_MASTER };

	OSStatus err = AudioObjectGetPropertyData(
			kAudioObjectSystemObject,
			&def_id_address, 0, NULL,
			&dev_property_size, &device_id);

	if (err != kAudioHardwareNoError)
	{
		osd_printf_error("CoreAudio: Error getting the default audio device.\n");
		return 0;
	}

	return device_id;
}

void sound_coreaudio::build_device_list()
{
	OSStatus err;
	UInt32 property_size;

	AudioObjectPropertyAddress const devices_addr = {
		kAudioHardwarePropertyDevices,
		kAudioObjectPropertyScopeGlobal,
		PROPERTY_ELEMENT_MASTER };

	err = AudioObjectGetPropertyDataSize(
			kAudioObjectSystemObject,
			&devices_addr,
			0,
			nullptr,
			&property_size);
	if (noErr != err)
	{
		osd_printf_error("CoreAudio: Error getting size of audio device list (%d)\n", err);
		return;
	}
	property_size /= sizeof(AudioDeviceID);
	std::unique_ptr<AudioDeviceID[]> const devices = std::make_unique<AudioDeviceID[]>(property_size);
	property_size *= sizeof(AudioDeviceID);
	err = AudioObjectGetPropertyData(
		kAudioObjectSystemObject,
		&devices_addr,
		0,
		nullptr,
		&property_size,
		devices.get());

	UInt32 const device_count = property_size / sizeof(AudioDeviceID);
	if (noErr != err)
	{
		osd_printf_error("CoreAudio: Error getting audio device list (%d)\n", err);
		return;
	}

	m_device_list.clear();

	m_deviceinfo.m_nodes.clear();
	m_deviceinfo.m_nodes.reserve(device_count);

	osd_printf_verbose("CoreAudio: Available devices are:\n");
	for (UInt32 i = 0; i < device_count; i++)
	{
		std::unique_ptr<char[]> const device_uid = get_device_uid(devices[i]);
		std::unique_ptr<char[]> const device_name = get_device_name(devices[i]);
		if (!device_uid && !device_name)
		{
			osd_printf_warning(
					"CoreAudio: Could not get UID or name for device %u - skipping\n",
					devices[i]);
			continue;
		}

		// only one of the two is guaranteed, so substitute the other for whichever is
		// missing - these must never be null, they end up in std::string
		char const *const uid = device_uid ? device_uid.get() : device_name.get();
		char const *const name = device_name ? device_name.get() : device_uid.get();

		UInt32 const in_streams = get_input_stream_count(devices[i], uid, name);
		UInt32 const out_streams = get_output_stream_count(devices[i], uid, name);

		// count the channels in each direction separately - a duplex device can present a
		// different number of channels for capture and playback
		int const source_channels = get_channel_count(devices[i], kAudioDevicePropertyScopeInput, name);
		int const sink_channels = get_channel_count(devices[i], kAudioDevicePropertyScopeOutput, name);

		// MAME models a node as either a sink or a source - audio_info::node_info::name()
		// picks the prefix purely from m_sinks - so describe a duplex device by its output
		// side, which is how the core will end up treating it
		bool const is_sink = (sink_channels > 0);
		AudioObjectPropertyScope const node_scope = is_sink ? kAudioDevicePropertyScopeOutput : kAudioDevicePropertyScopeInput;

		Float64 sample_rate;
		AudioObjectPropertyAddress const rate_addr = {
			kAudioDevicePropertyNominalSampleRate,
			node_scope,
			PROPERTY_ELEMENT_MASTER };

		UInt32 size = sizeof(Float64);
		err = AudioObjectGetPropertyData(
				devices[i],
				&rate_addr,
				0,
				nullptr,
				&size,
				&sample_rate);

		if (err != noErr)
		{
			osd_printf_error("CoreAudio: couldn't get sample rate for ID %d, defaulting to 44100\n", devices[i]);
			sample_rate = 44100.0f;
		}

		osd_printf_verbose("           %s (%s) ID %d supports %d input streams (%d channels) and %d output streams (%d channels), rate %d\n",
							name,
							uid,
							devices[i],
							in_streams,
							source_channels,
							out_streams,
							sink_channels,
							sample_rate);

		auto &node = m_deviceinfo.m_nodes.emplace_back();
		node.m_name = uid;
		node.m_display_name = name;
		node.m_id = devices[i];
		node.m_rate.m_default_rate = (int)sample_rate;
		// CoreAudio will resample on the way out, but capture is delivered at the device's rate
		node.m_rate.m_min_rate = is_sink ? 8000 : (int)sample_rate;
		node.m_rate.m_max_rate = is_sink ? 96000 : (int)sample_rate;
		node.m_sinks = sink_channels;
		node.m_sources = source_channels;
		node.m_port_names.clear();
		node.m_port_positions.clear();

		AudioObjectPropertyAddress const layout_addr = {
			kAudioDevicePropertyPreferredChannelLayout,
			node_scope,
			PROPERTY_ELEMENT_MASTER };

		AudioChannelLayout *chanLayout = (AudioChannelLayout *)nullptr;
		err = AudioObjectGetPropertyDataSize(
				devices[i],
				&layout_addr,
				0,
				nullptr,
				&size);

		if (err)
		{
			osd_printf_error("CoreAudio: couldn't get size of layout %d\n", err);
			size = sizeof(AudioChannelLayout) * 3;
		}
		else
		{
			chanLayout = (AudioChannelLayout *)malloc(size);
			err = AudioObjectGetPropertyData(
				devices[i],
				&layout_addr,
				0,
				nullptr,
				&size,
				chanLayout);

			if (err != noErr)
			{
				osd_printf_error("CoreAudio: couldn't get channel layout, %d\n", err);
			}
			else
			{
				osd_printf_verbose("\t\tlayout tag %x, bitmap %x, %d descriptions\n",
						chanLayout->mChannelLayoutTag,
						chanLayout->mChannelBitmap,
						chanLayout->mNumberChannelDescriptions);

				UInt32 descType = chanLayout->mChannelLayoutTag & kAudioChannelLayoutTag_UseChannelBitmap;
				if (!descType)  // bit clear = use channel descriptions
				{
					for (int desc = 0; desc < chanLayout->mNumberChannelDescriptions; desc++)
					{
						const auto &chDesc = chanLayout->mChannelDescriptions[desc];

						if ((chDesc.mChannelLabel == 0xffffffff) || (chDesc.mChannelLabel >= sMacChannelCount))
						{
							if ((chanLayout->mNumberChannelDescriptions > 1) && ((desc + 1) < sMacChannelCount))
							{
								node.m_port_names.push_back(sMacChannelLabels[desc + 1]);
								node.m_port_positions.emplace_back(sChannelPositions[desc + 1]);
							}
							else
							{
								std::string chLabel = "Channel " + std::to_string(desc + 1);
								node.m_port_names.push_back(chLabel);
								node.m_port_positions.emplace_back(osd::channel_position::FC());
							}
						}
						else
						{
							node.m_port_names.push_back(sMacChannelLabels[chDesc.mChannelLabel]);
							node.m_port_positions.emplace_back(sChannelPositions[chDesc.mChannelLabel]);
						}
					}

					for (int desc = 0; desc < chanLayout->mNumberChannelDescriptions; desc++)
					{
						const auto &chDesc = chanLayout->mChannelDescriptions[desc];

						osd_printf_verbose("\t\t\tch %d: flags %d label %s (%d) coords (%f %f %f)\n",
								desc,
								chDesc.mChannelFlags,
								node.m_port_names[desc].c_str(),
								chDesc.mChannelLabel,
								chDesc.mCoordinates[0],
								chDesc.mCoordinates[1],
								chDesc.mCoordinates[2]);
					}
				}
				else    // bit set, use channel bitmap
				{
					for (int channel = 0; channel < 32; channel++)
					{
						if (chanLayout->mChannelBitmap & (1U << channel))
						{
							// bits 0-17 map straight onto labels 1-18, but the bitmap has a gap at bits
							// 18-20 and resumes at bit 21 (Left Top Middle, label 49)
							const int chAdj = (21 <= channel)
									? (channel + (kAudioChannelLabel_LeftTopMiddle - 21))
									: (channel + 1);
							node.m_port_names.push_back(sMacChannelLabels[chAdj]);
							node.m_port_positions.emplace_back(sChannelPositions[chAdj]);
						}
					}
				}
			}
			free((void *)chanLayout);
		}

		m_device_list.emplace(
			devices[i],
			coreaudio_device(name, uid, devices[i], out_streams, in_streams, sink_channels, source_channels, sample_rate));
	}
}

UInt32 sound_coreaudio::get_input_stream_count(
	AudioDeviceID id,
	char const *uid,
	char const *name) const
{
	AudioObjectPropertyAddress const streams_addr = {
		kAudioDevicePropertyStreams,
		kAudioDevicePropertyScopeInput,
		PROPERTY_ELEMENT_MASTER };
	UInt32 property_size = 0;
	OSStatus const err = AudioObjectGetPropertyDataSize(
			id,
			&streams_addr,
			0,
			nullptr,
			&property_size);
	if (noErr != err)
	{
		osd_printf_warning(
				"CoreAudio: Error getting input stream count for audio device %s (%s) (%d)\n",
				(nullptr != name) ? name : "<anonymous>",
				(nullptr != uid) ? uid : "<unknown>",
				err);
		return 0;
	}
	return property_size / sizeof(AudioStreamID);
}

UInt32 sound_coreaudio::get_output_stream_count(
	AudioDeviceID id,
	char const *uid,
	char const *name) const
{
	AudioObjectPropertyAddress const streams_addr = {
		kAudioDevicePropertyStreams,
		kAudioDevicePropertyScopeOutput,
		PROPERTY_ELEMENT_MASTER };
	UInt32 property_size = 0;
	OSStatus const err = AudioObjectGetPropertyDataSize(
			id,
			&streams_addr,
			0,
			nullptr,
			&property_size);
	if (noErr != err)
	{
		osd_printf_warning(
				"CoreAudio: Error getting output stream count for audio device %s (%s) (%d)\n",
				(nullptr != name) ? name : "<anonymous>",
				(nullptr != uid) ? uid : "<unknown>",
				err);
		return 0;
	}
	return property_size / sizeof(AudioStreamID);
}

UInt32 sound_coreaudio::get_channel_count(
	AudioDeviceID id,
	AudioObjectPropertyScope scope,
	char const *name) const
{
	AudioObjectPropertyAddress const stream_config_addr = {
		kAudioDevicePropertyStreamConfiguration,
		scope,
		PROPERTY_ELEMENT_MASTER };

	UInt32 property_size = 0;
	OSStatus err = AudioObjectGetPropertyDataSize(
		id,
		&stream_config_addr,
		0,
		nullptr,
		&property_size);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: couldn't get stream config size for %s (%d)\n",
				(nullptr != name) ? name : "<anonymous>",
				err);
		return 0;
	}

	std::unique_ptr<uint8_t[]> const storage = std::make_unique<uint8_t[]>(property_size);
	AudioBufferList *const buffer_list = (AudioBufferList *)storage.get();
	err = AudioObjectGetPropertyData(
			id,
			&stream_config_addr,
			0,
			nullptr,
			&property_size,
			buffer_list);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: couldn't get stream configuration for %s (%d)\n",
				(nullptr != name) ? name : "<anonymous>",
				err);
		return 0;
	}

	UInt32 num_channels = 0;
	for (UInt32 buffer = 0; buffer < buffer_list->mNumberBuffers; buffer++)
	{
		num_channels += buffer_list->mBuffers[buffer].mNumberChannels;
	}
	return num_channels;
}

void sound_coreaudio::coreaudio_stream::close()
{
	if (m_render_errors || m_overflows || m_underflows)
	{
		osd_printf_verbose(
				"CoreAudio: Stream on device %d saw %u render error(s), %u overflow(s) and %u underflow(s)\n",
				m_id,
				m_render_errors.load(),
				m_overflows,
				m_underflows);
	}

	if (m_graph)
	{
		// stop the graph before locking m_stream_mutex, else AUGraphStop deadlocks against the render callback that also takes it
		AUGraphStop(m_graph);
		std::lock_guard<std::mutex> steam_guard(m_stream_mutex);
		AUGraphUninitialize(m_graph);
		DisposeAUGraph(m_graph);
		m_graph = nullptr;
		m_node_count = 0;
	}

	m_buffer.reset();
}

void sound_coreaudio::coreaudio_stream::sink_update(int16_t const *buffer, int samples_this_frame)
{
	if ((m_sample_rate == 0) || !m_buffer)
	{
		return;
	}

	uint32_t const bytes_this_frame = samples_this_frame * m_sample_bytes;
	if (bytes_this_frame >= buffer_avail())
	{
		m_overflows++;
		return;
	}

	uint32_t const chunk = std::min(m_buffer_size - m_writepos, bytes_this_frame);
	memcpy(&m_buffer[m_writepos], (int8_t *)buffer, chunk);
	m_writepos += chunk;
	if (m_writepos >= m_buffer_size)
		m_writepos = 0;

	if (chunk < bytes_this_frame)
	{
		assert(0U == m_writepos);
		assert(m_playpos > (bytes_this_frame - chunk));
		memcpy(&m_buffer[0], (int8_t *)buffer + chunk, bytes_this_frame - chunk);
		m_writepos += bytes_this_frame - chunk;
	}
}

bool sound_coreaudio::coreaudio_stream::create_sink_graph(struct coreaudio_device &device)
{
	OSStatus err;

	osd_printf_verbose("CoreAudio: Creating sink graph\n");
	if (noErr != (err = NewAUGraph(&m_graph)))
	{
		osd_printf_error("CoreAudio: Failed to create AudioUnit graph (%d)\n", err);
		goto return_error;
	}
	if (noErr != (err = AUGraphOpen(m_graph)))
	{
		osd_printf_error("CoreAudio: Failed to open AudioUnit graph (%d)\n", err);
		goto dispose_graph_and_return_error;
	}

	if (!add_device_output(device))
		goto close_graph_and_return_error;

	if ((1U < m_node_count) && !add_converter())
		goto close_graph_and_return_error;

	{
		AURenderCallbackStruct const renderer = { this->sink_render_callback, this };
		err = AUGraphSetNodeInputCallback(
			m_graph,
			m_node_details[m_node_count - 1].m_node,
			0,
			&renderer);
	}
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to set audio render callback for AudioUnit graph (%d)\n",
				err);
		goto close_graph_and_return_error;
	}

	err = AUGraphUpdate(m_graph, nullptr);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to update AudioUnit graph (%d)\n",
				err);
		goto close_graph_and_return_error;
	}
	osd_printf_verbose("CoreAudio: Sink graph created successfully\n");
	return true;

close_graph_and_return_error:
	AUGraphClose(m_graph);
dispose_graph_and_return_error:
	DisposeAUGraph(m_graph);
return_error:
	m_graph = nullptr;
	m_node_count = 0;
	return false;
}

bool sound_coreaudio::coreaudio_stream::create_source_graph(struct coreaudio_device &device)
{
	OSStatus err;
	UInt32 packet_size = 512;   // a small buffer keeps latency down, but we cope with any size
	UInt32 max_frames = 0;
	UInt32 property_size;
	AURenderCallbackStruct const renderer = { this->source_render_callback, this };
	AudioObjectPropertyAddress const packet_size_addr = {
		kAudioDevicePropertyBufferFrameSize,
		kAudioDevicePropertyScopeInput,
		PROPERTY_ELEMENT_MASTER };
	AudioObjectPropertyAddress const packet_range_addr = {
		kAudioDevicePropertyBufferFrameSizeRange,
		kAudioDevicePropertyScopeInput,
		PROPERTY_ELEMENT_MASTER };
	AudioValueRange packet_range = { 0.0, 0.0 };
	AudioStreamBasicDescription format;

	memset(&format, 0, sizeof(AudioStreamBasicDescription));

	osd_printf_verbose("CoreAudio: Creating source graph\n");
	if (noErr != (err = NewAUGraph(&m_graph)))
	{
		osd_printf_error("CoreAudio: Failed to create AudioUnit graph (%d)\n", err);
		goto return_error;
	}
	if (noErr != (err = AUGraphOpen(m_graph)))
	{
		osd_printf_error("CoreAudio: Failed to open AudioUnit graph (%d)\n", err);
		goto dispose_graph_and_return_error;
	}

	if (!add_device_input(device))
		goto close_graph_and_return_error;

	// The device's I/O buffer size determines how many frames AUHAL asks for in a single
	// callback.  Request a small one to keep latency down, but this is a property of the
	// device shared by every process using it, so the request may be refused or clamped and
	// another process can change it at any time.  Failing to set it is not fatal.
	property_size = sizeof(packet_range);
	err = AudioObjectGetPropertyData(
		device.m_id,
		&packet_range_addr,
		0,
		nullptr,
		&property_size,
		&packet_range);
	if (noErr == err)
		packet_size = std::clamp<UInt32>(packet_size, packet_range.mMinimum, packet_range.mMaximum);

	err = AudioObjectSetPropertyData(
		device.m_id,
		&packet_size_addr,
		0,
		nullptr,
		sizeof(packet_size),
		&packet_size);
	if (noErr != err)
	{
		osd_printf_verbose(
				"CoreAudio: Could not set input packet size (%d) - using the device's current setting\n",
				err);
	}

	format.mFormatID = kAudioFormatLinearPCM;
	format.mFormatFlags = 0U | kAudioFormatFlagsNativeEndian | kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked; // zero because C++20 doesn't allow arithmetic between different enum types
	format.mFramesPerPacket = 1;
	format.mChannelsPerFrame = device.m_source_channels;
	format.mBitsPerChannel = 16;
	format.mBytesPerFrame = format.mChannelsPerFrame * format.mBitsPerChannel / 8;
	format.mBytesPerPacket = format.mFramesPerPacket * format.mBytesPerFrame;
	format.mSampleRate = device.m_sample_rate;

	m_sample_bytes = format.mBytesPerFrame;

	err = AudioUnitSetProperty(
			m_node_details[m_node_count - 1].m_unit,
			kAudioUnitProperty_StreamFormat,
			kAudioUnitScope_Output,
			1,
			&format,
			sizeof(format));
	if (noErr != err)
	{
		osd_printf_error("CoreAudio: Could not set input stream format (%d)\n", err);
		goto close_graph_and_return_error;
	}

	err = AudioUnitSetProperty(
			m_node_details[m_node_count - 1].m_unit,
			kAudioOutputUnitProperty_SetInputCallback,
			kAudioUnitScope_Global,
			0,
			&renderer,
			sizeof(renderer));
	if (noErr != err)
	{
		osd_printf_error("CoreAudio: Could not set input callback (%d)\n", err);
		goto close_graph_and_return_error;
	}

	// Work out the largest render the system can ask us for.  MaximumFramesPerSlice reflects the
	// device's current I/O buffer size, but any process can enlarge that while we're running.
	// So also allow for the largest buffer the device will accept.
	m_max_frames = packet_size;
	property_size = sizeof(max_frames);
	err = AudioUnitGetProperty(
		m_node_details[m_node_count - 1].m_unit,
		kAudioUnitProperty_MaximumFramesPerSlice,
		kAudioUnitScope_Global,
		0,
		&max_frames,
		&property_size);
	if (noErr == err)
	{
		m_max_frames = std::max<uint32_t>(m_max_frames, max_frames);
	}
	if (packet_range.mMaximum > 0.0)
	{
		m_max_frames = std::max<uint32_t>(m_max_frames, packet_range.mMaximum);
	}

	osd_printf_verbose(
			"CoreAudio: Input I/O buffer is %u frames, sizing for renders of up to %u frames\n",
			packet_size,
			m_max_frames);

	err = AUGraphUpdate(m_graph, nullptr);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to update AudioUnit graph (%d)\n",
				err);
		goto close_graph_and_return_error;
	}

	osd_printf_verbose("CoreAudio: Source graph created successfully\n");
	return true;

close_graph_and_return_error:
	AUGraphClose(m_graph);
dispose_graph_and_return_error:
	DisposeAUGraph(m_graph);
return_error:
	m_graph = nullptr;
	m_node_count = 0;
	return false;
}

bool sound_coreaudio::coreaudio_stream::add_device_output(struct coreaudio_device &device)
{
	OSStatus err;
	const AudioDeviceID id = device.m_id;

	osd_printf_verbose("CoreAudio: Adding HAL output device %s to AudioUnit graph\n", device.m_name);
	err = add_node(
			kAudioUnitType_Output,
			kAudioUnitSubType_HALOutput,
			kAudioUnitManufacturer_Apple);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to add HAL output to AudioUnit graph (%d) - falling back to default output\n",
				err);
		return false;
	}
	if (noErr != (err = get_next_node_info()))
	{
		osd_printf_error(
			"CoreAudio: Failed to obtain AudioUnit for HAL output (%ld)\n",
			(long)err);
		goto remove_node_and_return_error;
	}
	err = AudioUnitSetProperty(
		m_node_details[m_node_count].m_unit,
		kAudioOutputUnitProperty_CurrentDevice,
		kAudioUnitScope_Global,
		0,
		&id,
		sizeof(id));
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to set HAL output device to %s (%d)\n",
				device.m_name,
				err);
		goto remove_node_and_return_error;
	}

	m_node_count++;
	return true;

remove_node_and_return_error:
	osd_printf_verbose("CoreAudio: Removing failed HAL output from AudioUnit graph\n");
	err = AUGraphRemoveNode(m_graph, m_node_details[m_node_count].m_node);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to remove HAL output from AudioUnit graph (%d)\n",
				err);
	}
	return false;
}

bool sound_coreaudio::coreaudio_stream::add_device_input(struct coreaudio_device &device)
{
	OSStatus err;
	UInt32 enable = 1;

	osd_printf_verbose("CoreAudio: Adding HAL input device %s to AudioUnit graph\n", device.m_name);
	err = add_node(
			kAudioUnitType_Output,
			kAudioUnitSubType_HALOutput,
			kAudioUnitManufacturer_Apple);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to add HAL input to AudioUnit graph (%d) - falling back to default output\n",
				err);
		return false;
	}
	if (noErr != (err = get_next_node_info()))
	{
		osd_printf_error(
				"CoreAudio: Failed to obtain AudioUnit for HAL input (%d)\n",
				err);
		goto remove_node_and_return_error;
	}

	// enable input and disable output
	err = AudioUnitSetProperty(
			m_node_details[m_node_count].m_unit,
			kAudioOutputUnitProperty_EnableIO,
			kAudioUnitScope_Input,
			1,
			&enable,
			sizeof(enable));
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to enable input on source stream (%d)\n",
				err);
		goto remove_node_and_return_error;
	}
	enable = 0;
	err = AudioUnitSetProperty(
			m_node_details[m_node_count].m_unit,
			kAudioOutputUnitProperty_EnableIO,
			kAudioUnitScope_Output,
			0,
			&enable,
			sizeof(enable));
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to disable output on source stream (%d)\n",
				err);
		goto remove_node_and_return_error;
	}

	// set the actual device - this has to happen after enabling I/O
	err = AudioUnitSetProperty(
			m_node_details[m_node_count].m_unit,
			kAudioOutputUnitProperty_CurrentDevice,
			kAudioUnitScope_Global,
			0,
			&device.m_id,
			sizeof(device.m_id));
	if (noErr != err)
	{
		osd_printf_error(
			"CoreAudio: Failed to set HAL input device to %s (%d)\n",
				device.m_name,
				err);
		goto remove_node_and_return_error;
	}

	m_node_count++;
	return true;

remove_node_and_return_error:
	osd_printf_verbose("CoreAudio: Removing failed HAL output from AudioUnit graph\n");
	err = AUGraphRemoveNode(m_graph, m_node_details[m_node_count].m_node);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to remove HAL output from AudioUnit graph (%d)\n",
				err);
	}
	return false;
}

bool sound_coreaudio::coreaudio_stream::add_converter()
{
	OSStatus err;
	err = add_node(
		kAudioUnitType_FormatConverter,
		kAudioUnitSubType_AUConverter,
		kAudioUnitManufacturer_Apple);
	if (noErr != err)
	{
		osd_printf_error(
				"CoreAudio: Failed to add sound format converter to AudioUnit graph (%d)\n",
				err);
		return false;
	}
	if (noErr != (err = get_next_node_info()))
	{
		osd_printf_error(
				"CoreAudio: Failed to obtain AudioUnit for sound format converter (%d)\n",
				err);
		return false;
	}
	if (noErr != (err = connect_next_node()))
	{
		osd_printf_error(
				"CoreAudio: Failed to connect sound format converter in AudioUnit graph (%d)\n",
				err);
		return false;
	}
	m_node_count++;
	return true;
}

OSStatus sound_coreaudio::coreaudio_stream::sink_render(
	AudioUnitRenderActionFlags *action_flags,
	const AudioTimeStamp *timestamp,
	UInt32 bus_number,
	UInt32 number_frames,
	AudioBufferList *data)
{
	std::lock_guard<std::mutex> stream_guard(m_stream_mutex);
	uint32_t const number_bytes = number_frames * m_sample_bytes;
	uint32_t const used = buffer_used();
	if (m_in_underrun && (used < m_headroom))
	{
		memset(data->mBuffers[0].mData, 0, number_bytes);
		return noErr;
	}
	m_in_underrun = false;
	if (number_bytes > used)
	{
		m_in_underrun = true;
		m_underflows++;
		memset(data->mBuffers[0].mData, 0, number_bytes);
		return noErr;
	}

	uint32_t const chunk = std::min(m_buffer_size - m_playpos, number_bytes);
	memcpy((int8_t *)data->mBuffers[0].mData, &m_buffer[m_playpos], chunk);
	m_playpos += chunk;
	if (m_playpos >= m_buffer_size)
		m_playpos = 0;

	if (chunk < number_bytes)
	{
		assert(0U == m_playpos);
		assert(m_writepos >= (number_bytes - chunk));
		memcpy((int8_t *)data->mBuffers[0].mData + chunk, &m_buffer[0], number_bytes - chunk);
		m_playpos += number_bytes - chunk;
	}

	return noErr;
}

OSStatus sound_coreaudio::coreaudio_stream::source_render(
	AudioUnitRenderActionFlags *action_flags,
	const AudioTimeStamp *timestamp,
	UInt32 bus_number,
	UInt32 number_frames,
	AudioBufferList *data)
{
	// The device's buffer size can be changed by any process while we're running, so
	// don't assume it still fits.
	uint32_t const number_bytes = number_frames * m_sample_bytes;
	if (!m_buffer || (number_bytes > m_buffer_size))
	{
		m_overflows++;
		return noErr;
	}

	AudioBufferList inputAudioBufferList;
	inputAudioBufferList.mNumberBuffers = 1;
	inputAudioBufferList.mBuffers[0].mNumberChannels = m_channels;
	inputAudioBufferList.mBuffers[0].mDataByteSize = number_bytes;
	inputAudioBufferList.mBuffers[0].mData = &m_buffer[0];

	OSStatus err = AudioUnitRender(
		m_node_details[0].m_unit,
		action_flags,
		timestamp,
		bus_number,
		number_frames,
		&inputAudioBufferList);
	if (err != noErr)
	{
		// this runs on the realtime I/O thread, so just count it and report on close
		m_render_errors++;
		return noErr;
	}
	else
	{
		// CoreAudio updates mDataByteSize with what it actually rendered, which may be less
		// than we asked for.  m_sample_bytes covers every channel in one frame.
		const int packets = inputAudioBufferList.mBuffers[0].mDataByteSize / m_sample_bytes;
		const int16_t *samples = (const int16_t *)&m_buffer[0];
		std::lock_guard<std::mutex> stream_guard(m_stream_mutex);
		m_input_buffer.push(samples, packets);
	}

	return noErr;
}

OSStatus sound_coreaudio::coreaudio_stream::sink_render_callback(
	void *refcon,
	AudioUnitRenderActionFlags *action_flags,
	const AudioTimeStamp *timestamp,
	UInt32 bus_number,
	UInt32 number_frames,
	AudioBufferList *data)
{
	return ((coreaudio_stream *)refcon)->sink_render(action_flags, timestamp, bus_number, number_frames, data);
}

OSStatus sound_coreaudio::coreaudio_stream::source_render_callback(
	void *refcon,
	AudioUnitRenderActionFlags *action_flags,
	const AudioTimeStamp *timestamp,
	UInt32 bus_number,
	UInt32 number_frames,
	AudioBufferList *data)
{
	return ((coreaudio_stream *)refcon)->source_render(action_flags, timestamp, bus_number, number_frames, data);
}

int sound_coreaudio::coreaudio_stream::create_sink_stream(struct coreaudio_device &device, const char *name, int sample_rate, float latency)
{
	OSErr err = noErr;

	m_audio_latency = latency;
	m_channels = device.m_sink_channels;
	m_sample_rate = sample_rate;
	m_id = device.m_id;
	m_is_source = false;

	// Create the output graph
	osd_printf_verbose("CoreAudio: Start create_sink_stream for %s (%d Hz, %d channels)\n", name, sample_rate, m_channels);
	if (!create_sink_graph(device))
		return -1;

	// Set audio stream format for native-endian 16-bit packed linear PCM
	AudioStreamBasicDescription format;
	format.mSampleRate = m_sample_rate;
	format.mFormatID = kAudioFormatLinearPCM;
	format.mFormatFlags = 0U | kAudioFormatFlagsNativeEndian | kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked; // zero because C++20 doesn't allow arithmetic between different enum types
	format.mFramesPerPacket = 1;
	format.mChannelsPerFrame = m_channels;
	format.mBitsPerChannel = 16;
	format.mBytesPerFrame = format.mChannelsPerFrame * format.mBitsPerChannel / 8;
	format.mBytesPerPacket = format.mFramesPerPacket * format.mBytesPerFrame;
	err = AudioUnitSetProperty(
		m_node_details[m_node_count - 1].m_unit,
		kAudioUnitProperty_StreamFormat,
		kAudioUnitScope_Input,
		0,
		&format,
		sizeof(format));

	if (noErr != err)
	{
		osd_printf_error("CoreAudio: Could not set audio output stream format (%ld)\n", (long)err);
		goto close_graph_and_return_error;
	}
	m_sample_bytes = format.mBytesPerFrame;

	// Allocate buffer
	m_headroom = m_sample_bytes * (m_audio_latency * m_sample_rate * 20e-3f);
	m_buffer_size = m_sample_bytes * std::max<uint32_t>(m_sample_rate * (m_audio_latency + 3) * 20e-3f, 512U);
	osd_printf_verbose("CoreAudio: Allocating %d bytes of buffer space (%d bytes per frame)\n", m_buffer_size, m_sample_bytes);
	try
	{
		m_buffer = std::make_unique<int8_t[]>(m_buffer_size);
	}
	catch (std::bad_alloc const &)
	{
		osd_printf_error("CoreAudio: Could not allocate sink stream buffer\n");
		goto close_graph_and_return_error;
	}
	m_playpos = 0;
	m_writepos = m_headroom;
	m_in_underrun = false;
	m_overflows = m_underflows = 0;

	// Initialise and start
	err = AUGraphInitialize(m_graph);
	if (noErr != err)
	{
		osd_printf_error("CoreAudio: Could not initialize AudioUnit graph (%ld)\n", (long)err);
		goto close_graph_and_return_error;
	}
	err = AUGraphStart(m_graph);
	if (noErr != err)
	{
		osd_printf_error("CoreAudio: Could not start AudioUnit graph (%ld)\n", (long)err);
		AUGraphUninitialize(m_graph);
		goto close_graph_and_return_error;
	}
	return 0;

close_graph_and_return_error:
	m_buffer_size = 0;
	m_buffer.reset();
	AUGraphClose(m_graph);
	DisposeAUGraph(m_graph);
	m_graph = nullptr;
	m_node_count = 0;
	return -1;
}

int sound_coreaudio::coreaudio_stream::create_source_stream(struct coreaudio_device &device, const char *name, int sample_rate, float latency)
{
	OSErr err = noErr;

	m_audio_latency = latency;
	m_channels = device.m_source_channels;
	m_sample_rate = sample_rate;
	m_id = device.m_id;
	m_is_source = true;

	// Create the input graph
	osd_printf_verbose("CoreAudio: Start create_source_stream for %s (%d Hz, %d channels)\n", name, sample_rate, m_channels);
	if (!create_source_graph(device))
		return -1;

	// Allocate 3 audio frames of buffer space for source streams, but never less than a
	// single render from the device.
	m_buffer_size = std::max<uint32_t>(m_max_frames, (m_sample_rate * 20e-3) * 3) * m_sample_bytes;
	m_input_buffer.set_latency(m_audio_latency);
	osd_printf_verbose("CoreAudio: Allocating %d bytes of source buffer space (%d bytes per frame)\n", m_buffer_size, m_sample_bytes);
	try
	{
		m_buffer = std::make_unique<int8_t[]>(m_buffer_size);
	}
	catch (std::bad_alloc const &)
	{
		osd_printf_error("CoreAudio: Could not allocate source stream buffer\n");
		goto close_graph_and_return_error;
	}

	// Initialise and start
	err = AUGraphInitialize(m_graph);
	if (noErr != err)
	{
		osd_printf_error("CoreAudio: Could not initialize AudioUnit graph (%ld)\n", (long)err);
		goto close_graph_and_return_error;
	}
	err = AUGraphStart(m_graph);
	if (noErr != err)
	{
		osd_printf_error("CoreAudio: Could not start AudioUnit graph (%ld)\n", (long)err);
		AUGraphUninitialize(m_graph);
		goto close_graph_and_return_error;
	}
	return 0;

close_graph_and_return_error:
	AUGraphClose(m_graph);
	DisposeAUGraph(m_graph);
	m_graph = nullptr;
	m_node_count = 0;
	return -1;
}

} // anonymous namespace

} // namespace osd

#else // SDLMAME_MACOSX

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_coreaudio, OSD_SOUND_PROVIDER, "coreaudio") } }

#endif // SDLMAME_MACOSX

MODULE_DEFINITION(SOUND_COREAUDIO, osd::sound_coreaudio)
