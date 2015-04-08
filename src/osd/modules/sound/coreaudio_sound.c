// license:BSD-3-Clause
// copyright-holders:Vas Crabb
//============================================================
//
//  sound.c - CoreAudio implementation of MAME sound routines
//
//  Copyright (c) 1996-2015, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//============================================================

#include "sound_module.h"
#include "modules/osdmodule.h"
#include "modules/lib/osdobj_common.h"

#ifdef SDLMAME_MACOSX

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include <string.h>


class sound_coreaudio : public osd_module, public sound_module
{
public:
	sound_coreaudio() :
		osd_module(OSD_SOUND_PROVIDER, "coreaudio"),
		sound_module(),
		m_graph(NULL),
		m_node_count(0),
		m_sample_bytes(0),
		m_headroom(0),
		m_buffer_size(0),
		m_buffer(NULL),
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

	virtual int init(osd_options const &options);
	virtual void exit();

	// sound_module

	virtual void update_audio_stream(bool is_throttled, INT16 const *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);

private:
	struct node_detail
	{
		node_detail() : m_node(0), m_unit(NULL) { }

		AUNode      m_node;
		AudioUnit   m_unit;
	};

	enum
	{
		LATENCY_MIN = 1,
		LATENCY_MAX = 5,
		EFFECT_COUNT_MAX = 10
	};

	UINT32 clamped_latency() const { return MAX(MIN(m_audio_latency, LATENCY_MAX), LATENCY_MIN); }
	UINT32 buffer_avail() const { return ((m_writepos >= m_playpos) ? m_buffer_size : 0) + m_playpos - m_writepos; }
	UINT32 buffer_used() const { return ((m_playpos > m_writepos) ? m_buffer_size : 0) + m_writepos - m_playpos; }

	void copy_scaled(void *dst, void const *src, UINT32 bytes) const
	{
		bytes /= sizeof(INT16);
		INT16 const *s = (INT16 const *)src;
		for (INT16 *d = (INT16 *)dst; bytes > 0; bytes--, s++, d++)
			*d = (*s * m_scale) >> 7;
	}

	bool create_graph(osd_options const &options);
	bool add_output();
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
				NULL,
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

	bool extract_effect_info(
			char const          *name,
			CFPropertyListRef   properties,
			OSType              &type,
			OSType              &subtype,
			OSType              &manufacturer,
			CFPropertyListRef   &class_info) const;
	CFPropertyListRef load_property_list(char const *name) const;

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

	UINT32      m_sample_bytes;
	UINT32      m_headroom;
	UINT32      m_buffer_size;
	INT8        *m_buffer;
	UINT32      m_playpos;
	UINT32      m_writepos;
	bool        m_in_underrun;
	INT32       m_scale;
	unsigned    m_overflows;
	unsigned    m_underflows;
};


int sound_coreaudio::init(const osd_options &options)
{
	OSStatus err;

	// Don't bother with any of this if sound is disabled
	if (sample_rate() == 0)
		return 0;

	// Create the output graph
	if (!create_graph(options))
		return -1;

	// Set audio stream format for two-channel native-endian 16-bit packed linear PCM
	AudioStreamBasicDescription format;
	format.mSampleRate          = sample_rate();
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
	m_headroom = m_sample_bytes * (clamped_latency() * sample_rate() / 40);
	m_buffer_size = m_sample_bytes * MAX(sample_rate() * (clamped_latency() + 3) / 40, 256);
	m_buffer = global_alloc_array_clear(INT8, m_buffer_size);
	if (!m_buffer)
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
		goto free_buffer_and_return_error;
	}
	err = AUGraphStart(m_graph);
	if (noErr != err)
	{
		osd_printf_error("Could not start AudioUnit graph (%ld)\n", (long)err);
		AUGraphUninitialize(m_graph);
		goto free_buffer_and_return_error;
	}
	osd_printf_verbose("Audio: End initialization\n");
	return 0;

free_buffer_and_return_error:
	global_free_array(m_buffer);
	m_buffer_size = 0;
	m_buffer = NULL;
close_graph_and_return_error:
	AUGraphClose(m_graph);
	DisposeAUGraph(m_graph);
	m_graph = NULL;
	m_node_count = 0;
	return -1;
}


void sound_coreaudio::exit()
{
	if (m_graph)
	{
		osd_printf_verbose("Stopping CoreAudio output\n");
		AUGraphStop(m_graph);
		AUGraphUninitialize(m_graph);
		DisposeAUGraph(m_graph);
		m_graph = NULL;
		m_node_count = 0;
	}
	if (m_buffer)
	{
		global_free_array(m_buffer);
		m_buffer = NULL;
	}
	if (m_overflows || m_underflows)
		osd_printf_verbose("Sound buffer: overflows=%u underflows=%u\n", m_overflows, m_underflows);
}


void sound_coreaudio::update_audio_stream(bool is_throttled, INT16 const *buffer, int samples_this_frame)
{
	if ((sample_rate() == 0) || !m_buffer)
		return;

	UINT32 const bytes_this_frame = samples_this_frame * m_sample_bytes;
	if (bytes_this_frame >= buffer_avail())
	{
		m_overflows++;
		return;
	}

	UINT32 const chunk = MIN(m_buffer_size - m_writepos, bytes_this_frame);
	memcpy(m_buffer + m_writepos, (INT8 *)buffer, chunk);
	m_writepos += chunk;
	if (m_writepos >= m_buffer_size)
		m_writepos = 0;

	if (chunk < bytes_this_frame)
	{
		assert(0U == m_writepos);
		assert(m_playpos > (bytes_this_frame - chunk));
		memcpy(m_buffer, (INT8 *)buffer + chunk, bytes_this_frame - chunk);
		m_writepos += bytes_this_frame - chunk;
	}
}


void sound_coreaudio::set_mastervolume(int attenuation)
{
	int const clamped_attenuation = MAX(MIN(attenuation, 0), -32);
	m_scale = (-32 == clamped_attenuation) ? 0 : (INT32)(pow(10.0, clamped_attenuation / 20.0) * 128);
}


bool sound_coreaudio::create_graph(osd_options const &options)
{
	OSStatus err;

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

	if (!add_output())
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

	err = AUGraphUpdate(m_graph, NULL);
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
	m_graph = NULL;
	m_node_count = 0;
	return false;
}


bool sound_coreaudio::add_output()
{
	OSStatus err;
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
	return true;
}


bool sound_coreaudio::add_converter()
{
	OSStatus err;
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
	if (NULL == properties)
		return true;

	OSType type, subtype, manufacturer;
	CFPropertyListRef class_info;
	if (!extract_effect_info(name, properties, type, subtype, manufacturer, class_info))
	{
		CFRelease(properties);
		return true;
	}

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
		err = AUParameterListenerNotify(NULL, NULL, &change);
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
	CFTypeRef type_val = NULL;
	CFTypeRef subtype_val = NULL;
	CFTypeRef manufacturer_val = NULL;
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
	if ((NULL == type_val)
	 || (NULL == subtype_val)
	 || (NULL == manufacturer_val)
	 || (NULL == class_info)
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
			NULL,
			(UInt8 const *)name,
			strlen(name),
			false);
	if (NULL == url)
	{
		return NULL;
	}

	CFDataRef data = NULL;
	SInt32 err;
	Boolean const status = CFURLCreateDataAndPropertiesFromResource(
			NULL,
			url,
			&data,
			NULL,
			NULL,
			&err);
	CFRelease(url);
	if (!status)
	{
		osd_printf_error(
				"Error reading data from %s (%ld)\n",
				name,
				(long)err);
		if (NULL != data) CFRelease(data);
		return NULL;
	}

	CFStringRef msg = NULL;
	CFPropertyListRef const result = CFPropertyListCreateFromXMLData(
			NULL,
			data,
			kCFPropertyListImmutable,
			&msg);
	CFRelease(data);
	if ((NULL == result) || (NULL != msg))
	{
		char *buf = NULL;
		if (NULL != msg)
		{
			CFIndex const len = CFStringGetMaximumSizeForEncoding(
					CFStringGetLength(msg),
					kCFStringEncodingUTF8);
			buf = global_alloc_array_clear(char, len + 1);
			if (!CFStringGetCString(msg, buf, len + 1, kCFStringEncodingUTF8))
			{
				global_free_array(buf);
				buf = NULL;
			}
			CFRelease(msg);
		}
		if (NULL != buf)
		{
			osd_printf_error(
					"Error creating property list from %s: %s\n",
					name,
					buf);
			global_free_array(buf);
		}
		else
		{
			osd_printf_error(
					"Error creating property list from %s\n",
					name);
		}
		if (NULL != result) CFRelease(result);
		return NULL;
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
	UINT32 const number_bytes = number_frames * m_sample_bytes;
	UINT32 const used = buffer_used();
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

	UINT32 const chunk = MIN(m_buffer_size - m_playpos, number_bytes);
	copy_scaled((INT8 *)data->mBuffers[0].mData, m_buffer + m_playpos, chunk);
	m_playpos += chunk;
	if (m_playpos >= m_buffer_size)
		m_playpos = 0;

	if (chunk < number_bytes)
	{
		assert(0U == m_playpos);
		assert(m_writepos >= (number_bytes - chunk));
		copy_scaled((INT8 *)data->mBuffers[0].mData + chunk, m_buffer, number_bytes - chunk);
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

#else /* SDLMAME_MACOSX */
	MODULE_NOT_SUPPORTED(sound_coreaudio, OSD_SOUND_PROVIDER, "coreaudio")
#endif

MODULE_DEFINITION(SOUND_COREAUDIO, sound_coreaudio)
