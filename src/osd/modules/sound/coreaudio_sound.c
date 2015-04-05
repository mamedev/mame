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

#ifdef SDLMAME_MACOSX

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreServices/CoreServices.h>


class sound_coreaudio : public osd_module, public sound_module
{
public:
	sound_coreaudio() :
		osd_module(OSD_SOUND_PROVIDER, "coreaudio"),
		sound_module(),
		m_open(false),
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

	virtual int init();
	virtual void exit();

	// sound_module

	virtual void update_audio_stream(bool is_throttled, INT16 const *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);

private:
	enum
	{
		LATENCY_MIN = 1,
		LATENCY_MAX = 5
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

	bool        m_open;
	AudioUnit   m_output;
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


int sound_coreaudio::init()
{
	OSStatus err;

	// Don't bother with any of this if sound is disabled
	if (sample_rate() == 0)
		return 0;

	// Get the Default Output AudioUnit component and open an instance
	ComponentDescription output_desc;
	output_desc.componentType           = kAudioUnitType_Output;
	output_desc.componentSubType        = kAudioUnitSubType_DefaultOutput;
	output_desc.componentManufacturer   = kAudioUnitManufacturer_Apple;
	output_desc.componentFlags          = 0;
	output_desc.componentFlagsMask      = 0;
	Component output_comp = FindNextComponent(NULL, &output_desc);
	if (!output_comp)
	{
		osd_printf_error("Could not find Default Output AudioUnit component\n");
		return -1;
	}
	err = OpenAComponent(output_comp, &m_output);
	if (noErr != err)
	{
		osd_printf_error("Could not open Default Output AudioUnit component (%ld)\n", (long)err);
		return -1;
	}

	// Set render callback
	AURenderCallbackStruct renderer;
	renderer.inputProc          = sound_coreaudio::render_callback;
	renderer.inputProcRefCon    = this;
	err = AudioUnitSetProperty(m_output, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &renderer, sizeof(renderer));
	if (noErr != err)
	{
		CloseComponent(m_output);
		osd_printf_error("Could not set audio output render callback (%ld)\n", (long)err);
		return -1;
	}

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
	err = AudioUnitSetProperty(m_output, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &format, sizeof(format));
	if (noErr != err)
	{
		CloseComponent(m_output);
		osd_printf_error("Could not set audio output stream format (%ld)\n", (long)err);
		return -1;
	}
	m_sample_bytes = format.mBytesPerFrame;

	// Allocate buffer
	m_headroom = m_sample_bytes * (clamped_latency() * sample_rate() / 40);
	m_buffer_size = m_sample_bytes * MAX(sample_rate() * (clamped_latency() + 3) / 40, 256);
	m_buffer = global_alloc_array_clear(INT8, m_buffer_size);
	if (!m_buffer)
	{
		CloseComponent(m_output);
		osd_printf_error("Could not allocate stream buffer\n");
		return -1;
	}
	m_playpos = 0;
	m_writepos = m_headroom;
	m_in_underrun = false;
	m_scale = 128;
	m_overflows = m_underflows = 0;

	// Initialise and start
	err = AudioUnitInitialize(m_output);
	if (noErr != err)
	{
		CloseComponent(m_output);
		global_free_array(m_buffer);
		m_buffer = NULL;
		osd_printf_error("Could not initialize audio output (%ld)\n", (long)err);
		return -1;
	}
	err = AudioOutputUnitStart(m_output);
	if (noErr != err)
	{
		AudioUnitUninitialize(m_output);
		CloseComponent(m_output);
		global_free_array(m_buffer);
		m_buffer = NULL;
		osd_printf_error("Could not start audio output (%ld)\n", (long)err);
		return -1;
	}
	m_open = true;
	osd_printf_verbose("Audio: End initialization\n");
	return 0;
}


void sound_coreaudio::exit()
{
	if (m_open)
	{
		osd_printf_verbose("Closing output AudioUnit component\n");
		AudioOutputUnitStop(m_output);
		AudioUnitUninitialize(m_output);
		CloseComponent(m_output);
		m_open = false;
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
