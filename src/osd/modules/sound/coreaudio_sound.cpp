// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  sound.c - CoreAudio implementation of MAME sound routines
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

#include <memory>
#include <new>
#include <cstring>


namespace osd {

namespace {

class sound_coreaudio : public osd_module, public sound_module
{
public:
	sound_coreaudio() :
		osd_module(OSD_SOUND_PROVIDER, "coreaudio"),
		sound_module(),
		m_graph(nullptr),
		m_node_count(0),
		m_sample_rate(0),
		m_audio_latency(0),
		m_sample_bytes(0),
		m_headroom(0),
		m_buffer_size(0),
		m_buffer(),
		m_playpos(0),
		m_writepos(0),
		m_in_underrun(false),
		m_scale(128),
		m_overflows(0),
		m_underflows(0)
	{
	}
	virtual ~sound_coreaudio()
	{
	}

	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override;

	// sound_module

	virtual void update_audio_stream(bool is_throttled, int16_t const *buffer, int samples_this_frame) override;
	virtual void set_mastervolume(int attenuation) override;

private:
	struct node_detail
	{
		node_detail() : m_node(0), m_unit(nullptr) { }

		AUNode      m_node;
		AudioUnit   m_unit;
	};

	enum
	{
		LATENCY_MIN = 1,
		LATENCY_MAX = 5,
		EFFECT_COUNT_MAX = 10
	};

	uint32_t clamped_latency() const { return unsigned(std::clamp<int>(m_audio_latency, LATENCY_MIN, LATENCY_MAX)); }
	uint32_t buffer_avail() const { return ((m_writepos >= m_playpos) ? m_buffer_size : 0) + m_playpos - m_writepos; }
	uint32_t buffer_used() const { return ((m_playpos > m_writepos) ? m_buffer_size : 0) + m_writepos - m_playpos; }

	void copy_scaled(void *dst, void const *src, uint32_t bytes) const
	{
		bytes /= sizeof(int16_t);
		int16_t const *s = (int16_t const *)src;
		for (int16_t *d = (int16_t *)dst; bytes > 0; bytes--, s++, d++)
			*d = (*s * m_scale) >> 7;
	}

	bool create_graph(osd_options const &options);
	bool add_output(char const *name);
	bool add_device_output(char const *name);
	bool add_converter();
	bool add_effect(char const *name);

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

	bool get_output_device_id(char const *name, AudioDeviceID &id) const;
	std::unique_ptr<char []> get_device_uid(AudioDeviceID id) const;
	std::unique_ptr<char []> get_device_name(AudioDeviceID id) const;
	UInt32 get_output_stream_count(
			AudioDeviceID id,
			char const *uid,
			char const *name) const;

	bool extract_effect_info(
			char const          *name,
			CFPropertyListRef   properties,
			OSType              &type,
			OSType              &subtype,
			OSType              &manufacturer,
			CFPropertyListRef   &class_info) const;
	CFPropertyListRef load_property_list(char const *name) const;

	std::unique_ptr<char []> convert_cfstring_to_utf8(CFStringRef str) const
	{
		CFIndex const len = CFStringGetMaximumSizeForEncoding(
				CFStringGetLength(str),
				kCFStringEncodingUTF8);
		std::unique_ptr<char []> result = std::make_unique<char []>(len + 1);
		if (!CFStringGetCString(str, result.get(), len + 1, kCFStringEncodingUTF8))
			result.reset();
		return result;
	}

	OSStatus render(
			AudioUnitRenderActionFlags  *action_flags,
			const AudioTimeStamp        *timestamp,
			UInt32                      bus_number,
			UInt32                      number_frames,
			AudioBufferList             *data);

	static OSStatus render_callback(
			void                        *refcon,
			AudioUnitRenderActionFlags  *action_flags,
			const AudioTimeStamp        *timestamp,
			UInt32                      bus_number,
			UInt32                      number_frames,
			AudioBufferList             *data);

	AUGraph     m_graph;
	unsigned    m_node_count;
	node_detail m_node_details[EFFECT_COUNT_MAX + 2];

	int         m_sample_rate;
	int         m_audio_latency;
	uint32_t    m_sample_bytes;
	uint32_t    m_headroom;
	uint32_t    m_buffer_size;
	std::unique_ptr<int8_t []> m_buffer;
	uint32_t    m_playpos;
	uint32_t    m_writepos;
	bool        m_in_underrun;
	int32_t     m_scale;
	unsigned    m_overflows;
	unsigned    m_underflows;
};


int sound_coreaudio::init(osd_interface &osd, const osd_options &options)
{
	OSStatus err;

	// Don't bother with any of this if sound is disabled
	m_sample_rate = options.sample_rate();
	m_audio_latency = options.audio_latency();
	if (m_sample_rate == 0)
		return 0;

	// Create the output graph
	osd_printf_verbose("Audio: Start initialization\n");
	if (!create_graph(options))
		return -1;

	// Set audio stream format for two-channel native-endian 16-bit packed linear PCM
	AudioStreamBasicDescription format;
	format.mSampleRate          = m_sample_rate;
	format.mFormatID            = kAudioFormatLinearPCM;
	format.mFormatFlags         = kAudioFormatFlagsNativeEndian
								| kLinearPCMFormatFlagIsSignedInteger
								| kLinearPCMFormatFlagIsPacked;
	format.mFramesPerPacket     = 1;
	format.mChannelsPerFrame    = 2;
	format.mBitsPerChannel      = 16;
	format.mBytesPerFrame       = format.mChannelsPerFrame * format.mBitsPerChannel / 8;
	format.mBytesPerPacket      = format.mFramesPerPacket * format.mBytesPerFrame;
	err = AudioUnitSetProperty(
			m_node_details[m_node_count - 1].m_unit,
			kAudioUnitProperty_StreamFormat,
			kAudioUnitScope_Input,
			0,
			&format,
			sizeof(format));
	if (noErr != err)
	{
		osd_printf_error("Could not set audio output stream format (%ld)\n", (long)err);
		goto close_graph_and_return_error;
	}
	m_sample_bytes = format.mBytesPerFrame;

	// Allocate buffer
	m_headroom = m_sample_bytes * (clamped_latency() * m_sample_rate / 40);
	m_buffer_size = m_sample_bytes * std::max<uint32_t>(m_sample_rate * (clamped_latency() + 3) / 40, 256U);
	try
	{
		m_buffer = std::make_unique<int8_t []>(m_buffer_size);
	}
	catch (std::bad_alloc const &)
	{
		osd_printf_error("Could not allocate stream buffer\n");
		goto close_graph_and_return_error;
	}
	m_playpos = 0;
	m_writepos = m_headroom;
	m_in_underrun = false;
	m_scale = 128;
	m_overflows = m_underflows = 0;

	// Initialise and start
	err = AUGraphInitialize(m_graph);
	if (noErr != err)
	{
		osd_printf_error("Could not initialize AudioUnit graph (%ld)\n", (long)err);
		goto close_graph_and_return_error;
	}
	err = AUGraphStart(m_graph);
	if (noErr != err)
	{
		osd_printf_error("Could not start AudioUnit graph (%ld)\n", (long)err);
		AUGraphUninitialize(m_graph);
		goto close_graph_and_return_error;
	}
	osd_printf_verbose("Audio: End initialization\n");
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


void sound_coreaudio::exit()
{
	osd_printf_verbose("Audio: Start deinitialization\n");
	if (m_graph)
	{
		osd_printf_verbose("Stopping CoreAudio output\n");
		AUGraphStop(m_graph);
		AUGraphUninitialize(m_graph);
		DisposeAUGraph(m_graph);
		m_graph = nullptr;
		m_node_count = 0;
	}
	m_buffer.reset();
	if (m_overflows || m_underflows)
		osd_printf_verbose("Sound buffer: overflows=%u underflows=%u\n", m_overflows, m_underflows);
	osd_printf_verbose("Audio: End deinitialization\n");
}


void sound_coreaudio::update_audio_stream(bool is_throttled, int16_t const *buffer, int samples_this_frame)
{
	if ((m_sample_rate == 0) || !m_buffer)
		return;

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


void sound_coreaudio::set_mastervolume(int attenuation)
{
	int const clamped_attenuation = std::clamp(attenuation, -32, 0);
	m_scale = (-32 == clamped_attenuation) ? 0 : (int32_t)(pow(10.0, clamped_attenuation / 20.0) * 128);
}


bool sound_coreaudio::create_graph(osd_options const &options)
{
	OSStatus err;

	osd_printf_verbose("Creating AudioUnit graph\n");
	if (noErr != (err = NewAUGraph(&m_graph)))
	{
		osd_printf_error("Failed to create AudioUnit graph (%ld)\n", (long)err);
		goto return_error;
	}
	if (noErr != (err = AUGraphOpen(m_graph)))
	{
		osd_printf_error("Failed to open AudioUnit graph (%ld)\n", (long)err);
		goto dispose_graph_and_return_error;
	}

	if (!add_output(options.audio_output()))
		goto close_graph_and_return_error;

	for (unsigned i = EFFECT_COUNT_MAX; 0U < i; i--)
	{
		if (!add_effect(options.audio_effect(i - 1)))
			goto close_graph_and_return_error;
	}

	if ((1U < m_node_count) && !add_converter())
		goto close_graph_and_return_error;

	{
		AURenderCallbackStruct const renderer = { sound_coreaudio::render_callback, this };
		err = AUGraphSetNodeInputCallback(
				m_graph,
				m_node_details[m_node_count - 1].m_node,
				0,
				&renderer);
	}
	if (noErr != err)
	{
		osd_printf_error(
				"Failed to set audio render callback for AudioUnit graph (%ld)\n",
				(long)err);
		goto close_graph_and_return_error;
	}

	err = AUGraphUpdate(m_graph, nullptr);
	if (noErr != err)
	{
		osd_printf_error(
				"Failed to update AudioUnit graph (%ld)\n",
				(long)err);
		goto close_graph_and_return_error;
	}

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


bool sound_coreaudio::add_output(char const *name)
{
	OSStatus err;

	if (*name && strcmp(name, OSDOPTVAL_AUTO) && !add_device_output(name))
		return false;

	if (0U == m_node_count)
	{
		osd_printf_verbose("Adding default output to AudioUnit graph\n");
		err = add_node(
				kAudioUnitType_Output,
				kAudioUnitSubType_DefaultOutput,
				kAudioUnitManufacturer_Apple);
		if (noErr != err)
		{
			osd_printf_error(
					"Failed to add default sound output to AudioUnit graph (%ld)\n",
					(long)err);
			return false;
		}
		if (noErr != (err = get_next_node_info()))
		{
			osd_printf_error(
					"Failed to obtain AudioUnit for default sound output (%ld)\n",
					(long)err);
			return false;
		}
		m_node_count++;
	}

	return true;
}


bool sound_coreaudio::add_device_output(char const *name)
{
	OSStatus err;

	AudioDeviceID id;
	if (!get_output_device_id(name, id))
	{
		osd_printf_warning(
				"No audio output device matched %s - falling back to default output\n",
				name);
		return true;
	}

	osd_printf_verbose("Adding HAL output to AudioUnit graph\n");
	err = add_node(
			kAudioUnitType_Output,
			kAudioUnitSubType_HALOutput,
			kAudioUnitManufacturer_Apple);
	if (noErr != err)
	{
		osd_printf_error(
				"Failed to add HAL output to AudioUnit graph (%ld) - falling back to default output\n",
				(long)err);
		return true;
	}
	if (noErr != (err = get_next_node_info()))
	{
		osd_printf_error(
				"Failed to obtain AudioUnit for HAL output (%ld)\n",
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
	if (noErr != (err = get_next_node_info()))
	{
		osd_printf_error(
				"Failed to set HAL output device to %s (%ld)\n",
				name,
				(long)err);
		goto remove_node_and_return_error;
	}
	m_node_count++;
	return true;

remove_node_and_return_error:
	osd_printf_verbose("Removing failed HAL output from AudioUnit graph\n");
	err = AUGraphRemoveNode(m_graph, m_node_details[m_node_count].m_node);
	if (noErr != err)
	{
		osd_printf_error(
				"Failed to remove HAL output from AudioUnit graph (%ld)\n",
				(long)err);
		return false;
	}
	osd_printf_warning("Falling back to default output");
	return true;
}


bool sound_coreaudio::add_converter()
{
	OSStatus err;
	osd_printf_verbose("Adding format converter to AudioUnit graph\n");
	err = add_node(
			kAudioUnitType_FormatConverter,
			kAudioUnitSubType_AUConverter,
			kAudioUnitManufacturer_Apple);
	if (noErr != err)
	{
		osd_printf_error(
				"Failed to add sound format converter to AudioUnit graph (%ld)\n",
				(long)err);
		return false;
	}
	if (noErr != (err = get_next_node_info()))
	{
		osd_printf_error(
				"Failed to obtain AudioUnit for sound format converter (%ld)\n",
				(long)err);
		return false;
	}
	if (noErr != (err = connect_next_node()))
	{
		osd_printf_error(
				"Failed to connect sound format converter in AudioUnit graph (%ld)\n",
				(long)err);
		return false;
	}
	m_node_count++;
	return true;
}


bool sound_coreaudio::add_effect(char const *name)
{
	OSStatus err;

	if (!*name || !strcmp(name, OSDOPTVAL_NONE))
		return true;

	CFPropertyListRef const properties = load_property_list(name);
	if (nullptr == properties)
		return true;

	OSType type, subtype, manufacturer;
	CFPropertyListRef class_info;
	if (!extract_effect_info(name, properties, type, subtype, manufacturer, class_info))
	{
		CFRelease(properties);
		return true;
	}

	osd_printf_verbose("Adding effect %s to AudioUnit graph\n", name);
	if (noErr != (err = add_node(type, subtype, manufacturer)))
	{
		osd_printf_error(
				"Failed to add effect %s to AudioUnit graph (%ld)\n",
				name,
				(long)err);
		CFRelease(properties);
		return true;
	}
	if (noErr != (err = get_next_node_info()))
	{
		osd_printf_error(
				"Failed to obtain AudioUnit for effect %s (%ld)\n",
				name,
				(long)err);
		CFRelease(properties);
		goto remove_node_and_return_error;
	}
	err = AudioUnitSetProperty(
			m_node_details[m_node_count].m_unit,
			kAudioUnitProperty_ClassInfo,
			kAudioUnitScope_Global,
			0,
			&class_info,
			sizeof(class_info));
	CFRelease(properties);
	if (noErr != err)
	{
		osd_printf_error(
				"Failed to configure AudioUnit effect %s (%ld)\n",
				name,
				(long)err);
		goto remove_node_and_return_error;
	}
	{
		AudioUnitParameter const change = {
				m_node_details[m_node_count].m_unit,
				kAUParameterListener_AnyParameter,
				0,
				0 };
		err = AUParameterListenerNotify(nullptr, nullptr, &change);
	}
	if (noErr != err)
	{
		osd_printf_error(
				"Failed to notify AudioUnit effect %s parameter change (%ld)\n",
				name,
				(long)err);
		goto remove_node_and_return_error;
	}
	if (noErr != (err = connect_next_node()))
	{
		osd_printf_error(
				"Failed to connect effect %s in AudioUnit graph (%ld)\n",
				name,
				(long)err);
		goto remove_node_and_return_error;
	}
	m_node_count++;
	return true;

remove_node_and_return_error:
	osd_printf_verbose("Removing failed effect %s from AudioUnit graph\n", name);
	err = AUGraphRemoveNode(m_graph, m_node_details[m_node_count].m_node);
	if (noErr != err)
	{
		osd_printf_error(
				"Failed to remove effect %s from AudioUnit graph (%ld)\n",
				name,
				(long)err);
		return false;
	}
	return true;
}


bool sound_coreaudio::get_output_device_id(
		char const *name,
		AudioDeviceID &id) const
{
	OSStatus err;
	UInt32 property_size;

	AudioObjectPropertyAddress const devices_addr = {
			kAudioHardwarePropertyDevices,
			kAudioObjectPropertyScopeGlobal,
			kAudioObjectPropertyElementMaster };
	err = AudioObjectGetPropertyDataSize(
			kAudioObjectSystemObject,
			&devices_addr,
			0,
			nullptr,
			&property_size);
	if (noErr != err)
	{
		osd_printf_error("Error getting size of audio device list (%ld)\n", (long)err);
		return false;
	}
	property_size /= sizeof(AudioDeviceID);
	std::unique_ptr<AudioDeviceID []> const devices = std::make_unique<AudioDeviceID []>(property_size);
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
		osd_printf_error("Error getting audio device list (%ld)\n", (long)err);
		return false;
	}

	for (UInt32 i = 0; device_count > i; i++)
	{
		std::unique_ptr<char []> const device_uid = get_device_uid(devices[i]);
		std::unique_ptr<char []> const device_name = get_device_name(devices[i]);
		if (!device_uid && !device_name)
		{
			osd_printf_warning(
					"Could not get UID or name for device %lu - skipping\n",
					(unsigned long)devices[i]);
			continue;
		}

		UInt32 const streams = get_output_stream_count(
				devices[i],
				device_uid.get(),
				device_name.get());
		if (1U > streams)
		{
			osd_printf_verbose(
					"No output streams found for device %s (%s) - skipping\n",
					device_name ? device_name.get() : "<anonymous>",
					device_uid ? device_uid.get() : "<unknown>");
			continue;
		}

		for (std::size_t j = strlen(device_uid.get()); (0 < j) && (' ' == device_uid[j - 1]); j--)
			device_uid[j - 1] = '\0';
		for (std::size_t j = strlen(device_name.get()); (0 < j) && (' ' == device_name[j - 1]); j--)
			device_name[j - 1] = '\0';

		bool const matched_uid = device_uid && !strcmp(name, device_uid.get());
		bool const matched_name = device_name && !strcmp(name, device_name.get());
		if (matched_uid || matched_name)
		{
			osd_printf_verbose(
					"Matched device %s (%s) with %lu output stream(s)\n",
					device_name ? device_name.get() : "<anonymous>",
					device_uid ? device_uid.get() : "<unknown>",
					(unsigned long)streams);
		}

		if (matched_uid || matched_name)
		{
			id = devices[i];
			return true;
		}
	}

	osd_printf_verbose("No audio output devices match %s\n", name);
	return false;
}


std::unique_ptr<char []> sound_coreaudio::get_device_uid(AudioDeviceID id) const
{
	AudioObjectPropertyAddress const uid_addr = {
			kAudioDevicePropertyDeviceUID,
			kAudioObjectPropertyScopeGlobal,
			kAudioObjectPropertyElementMaster };
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
				"Error getting UID for audio device %lu (%ld)\n",
				(unsigned long)id,
				(long)err);
		return nullptr;
	}
	std::unique_ptr<char []> result = convert_cfstring_to_utf8(device_uid);
	CFRelease(device_uid);
	if (!result)
	{
		osd_printf_warning(
				"Error converting UID for audio device %lu to UTF-8\n",
				(unsigned long)id);
	}
	return result;
}


std::unique_ptr<char []> sound_coreaudio::get_device_name(AudioDeviceID id) const
{
	AudioObjectPropertyAddress const name_addr = {
			kAudioDevicePropertyDeviceNameCFString,
			kAudioObjectPropertyScopeGlobal,
			kAudioObjectPropertyElementMaster };
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
				"Error getting name for audio device %lu (%ld)\n",
				(unsigned long)id,
				(long)err);
		return nullptr;
	}
	std::unique_ptr<char []> result = convert_cfstring_to_utf8(device_name);
	CFRelease(device_name);
	if (!result)
	{
		osd_printf_warning(
				"Error converting name for audio device %lu to UTF-8\n",
				(unsigned long)id);
	}
	return result;
}


UInt32 sound_coreaudio::get_output_stream_count(
		AudioDeviceID id,
		char const *uid,
		char const *name) const
{
	AudioObjectPropertyAddress const streams_addr = {
			kAudioDevicePropertyStreams,
			kAudioDevicePropertyScopeOutput,
			kAudioObjectPropertyElementMaster };
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
				"Error getting output stream count for audio device %s (%s) (%ld)\n",
				(nullptr != name) ? name : "<anonymous>",
				(nullptr != uid) ? uid : "<unknown>",
				(long)err);
		return 0;
	}
	return property_size / sizeof(AudioStreamID);
}


bool sound_coreaudio::extract_effect_info(
		char const          *name,
		CFPropertyListRef   properties,
		OSType              &type,
		OSType              &subtype,
		OSType              &manufacturer,
		CFPropertyListRef   &class_info) const
{
	if (CFDictionaryGetTypeID() != CFGetTypeID(properties))
	{
		osd_printf_error(
				"%s is not a valid AudioUnit effect description: expected dictionary\n",
				name);
		return false;
	}

	CFDictionaryRef const desc = (CFDictionaryRef)properties;
	CFTypeRef type_val = nullptr;
	CFTypeRef subtype_val = nullptr;
	CFTypeRef manufacturer_val = nullptr;
	if (CFDictionaryContainsKey(desc, CFSTR("ComponentType"))
		&& CFDictionaryContainsKey(desc, CFSTR("ComponentSubType"))
		&& CFDictionaryContainsKey(desc, CFSTR("ComponentManufacturer"))
		&& CFDictionaryContainsKey(desc, CFSTR("ClassInfo")))
	{
		type_val = CFDictionaryGetValue(desc, CFSTR("ComponentType"));
		subtype_val = CFDictionaryGetValue(desc, CFSTR("ComponentSubType"));
		manufacturer_val = CFDictionaryGetValue(desc, CFSTR("ComponentManufacturer"));
		class_info = CFDictionaryGetValue(desc, CFSTR("ClassInfo"));
	}
	else if (CFDictionaryContainsKey(desc, CFSTR(kAUPresetTypeKey))
			&& CFDictionaryContainsKey(desc, CFSTR(kAUPresetSubtypeKey))
			&& CFDictionaryContainsKey(desc, CFSTR(kAUPresetManufacturerKey)))
	{
		type_val = CFDictionaryGetValue(desc, CFSTR(kAUPresetTypeKey));
		subtype_val = CFDictionaryGetValue(desc, CFSTR(kAUPresetSubtypeKey));
		manufacturer_val = CFDictionaryGetValue(desc, CFSTR(kAUPresetManufacturerKey));
		class_info = properties;
	}
	else
	{
		osd_printf_error(
				"%s is not a valid AudioUnit effect description: required properties not found\n",
				name);
		return false;
	}

	SInt64 type_int, subtype_int, manufacturer_int;
	if ((nullptr == type_val)
		|| (nullptr == subtype_val)
		|| (nullptr == manufacturer_val)
		|| (nullptr == class_info)
		|| (CFNumberGetTypeID() != CFGetTypeID(type_val))
		|| (CFNumberGetTypeID() != CFGetTypeID(subtype_val))
		|| (CFNumberGetTypeID() != CFGetTypeID(manufacturer_val))
		|| (CFDictionaryGetTypeID() != CFGetTypeID(class_info))
		|| !CFNumberGetValue((CFNumberRef)type_val, kCFNumberSInt64Type, &type_int)
		|| !CFNumberGetValue((CFNumberRef)subtype_val, kCFNumberSInt64Type, &subtype_int)
		|| !CFNumberGetValue((CFNumberRef)manufacturer_val, kCFNumberSInt64Type, &manufacturer_int))
	{
		osd_printf_error(
				"%s is not a valid AudioUnit effect description: incorrect property type(s)\n",
				name);
		return false;
	}

	if (kAudioUnitType_Effect != type_int)
	{
		osd_printf_error(
				"%s does not describe an AudioUnit effect (type %lu, expected %lu)\n",
				name,
				(unsigned long)type_int,
				(unsigned long)kAudioUnitType_Effect);
		return false;
	}

	type = (OSType)(UInt64)type_int;
	subtype = (OSType)(UInt64)subtype_int;
	manufacturer = (OSType)(UInt64)manufacturer_int;
	return true;
}


CFPropertyListRef sound_coreaudio::load_property_list(char const *name) const
{
	CFURLRef const url = CFURLCreateFromFileSystemRepresentation(
			nullptr,
			(UInt8 const *)name,
			strlen(name),
			false);
	if (!url)
		return nullptr;

	CFReadStreamRef const stream = CFReadStreamCreateWithFile(nullptr, url);
	CFRelease(url);
	if (!stream)
	{
		osd_printf_error("Error opening file %s\n", name);
		return nullptr;
	}
	if (!CFReadStreamOpen(stream))
	{
		CFRelease(stream);
		osd_printf_error("Error opening file %s\n", name);
		return nullptr;
	}

	CFErrorRef msg = nullptr;
	CFPropertyListRef const result = CFPropertyListCreateWithStream(
			nullptr,
			stream,
			0,
			kCFPropertyListImmutable,
			nullptr,
			&msg);
	CFReadStreamClose(stream);
	CFRelease(stream);
	if (!result || msg)
	{
		CFStringRef const desc = msg ? CFErrorCopyDescription(msg) : nullptr;
		std::unique_ptr<char []> const buf = desc ? convert_cfstring_to_utf8(desc) : nullptr;
		if (desc)
			CFRelease(desc);
		if (msg)
			CFRelease(msg);

		if (buf)
		{
			osd_printf_error(
					"Error creating property list from %s: %s\n",
					name,
					buf.get());
		}
		else
		{
			osd_printf_error(
					"Error creating property list from %s\n",
					name);
		}
		if (result)
			CFRelease(result);
		return nullptr;
	}

	return result;
}


OSStatus sound_coreaudio::render(
		AudioUnitRenderActionFlags  *action_flags,
		const AudioTimeStamp        *timestamp,
		UInt32                      bus_number,
		UInt32                      number_frames,
		AudioBufferList             *data)
{
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
	copy_scaled((int8_t *)data->mBuffers[0].mData, &m_buffer[m_playpos], chunk);
	m_playpos += chunk;
	if (m_playpos >= m_buffer_size)
		m_playpos = 0;

	if (chunk < number_bytes)
	{
		assert(0U == m_playpos);
		assert(m_writepos >= (number_bytes - chunk));
		copy_scaled((int8_t *)data->mBuffers[0].mData + chunk, &m_buffer[0], number_bytes - chunk);
		m_playpos += number_bytes - chunk;
	}

	return noErr;
}


OSStatus sound_coreaudio::render_callback(
		void                        *refcon,
		AudioUnitRenderActionFlags  *action_flags,
		const AudioTimeStamp        *timestamp,
		UInt32                      bus_number,
		UInt32                      number_frames,
		AudioBufferList             *data)
{
	return ((sound_coreaudio *)refcon)->render(action_flags, timestamp, bus_number, number_frames, data);
}

} // anonymous namespace

} // namespace osd

#else // SDLMAME_MACOSX

namespace osd { namespace { MODULE_NOT_SUPPORTED(sound_coreaudio, OSD_SOUND_PROVIDER, "coreaudio") } }

#endif // SDLMAME_MACOSX

MODULE_DEFINITION(SOUND_COREAUDIO, osd::sound_coreaudio)
