// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#pragma once

#ifndef __K051649_H__
#define __K051649_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_K051649_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, K051649, _clock)
#define MCFG_K051649_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, K051649, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// Parameters for a channel
struct k051649_sound_channel
{
	k051649_sound_channel() :
		counter(0),
		frequency(0),
		volume(0),
		key(0)
	{
		memset(waveram, 0, sizeof(signed char)*32);
	}

	unsigned long counter;
	int frequency;
	int volume;
	int key;
	signed char waveram[32];
};


// ======================> k051649_device

class k051649_device : public device_t,
						public device_sound_interface
{
public:
	k051649_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k051649_device() { }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE8_MEMBER( k051649_waveform_w );
	DECLARE_READ8_MEMBER ( k051649_waveform_r );
	DECLARE_WRITE8_MEMBER( k051649_volume_w );
	DECLARE_WRITE8_MEMBER( k051649_frequency_w );
	DECLARE_WRITE8_MEMBER( k051649_keyonoff_w );
	DECLARE_WRITE8_MEMBER( k051649_test_w );
	DECLARE_READ8_MEMBER ( k051649_test_r );

	DECLARE_WRITE8_MEMBER( k052539_waveform_w );
	DECLARE_READ8_MEMBER ( k052539_waveform_r );

private:
	void make_mixer_table(int voices);

private:
	k051649_sound_channel m_channel_list[5];

	/* global sound parameters */
	sound_stream *m_stream;
	int m_mclock;
	int m_rate;

	/* mixer tables and internal buffers */
	INT16 *m_mixer_table;
	INT16 *m_mixer_lookup;
	short *m_mixer_buffer;

	/* chip registers */
	UINT8 m_test;
};

extern const device_type K051649;


#endif /* __K051649_H__ */
