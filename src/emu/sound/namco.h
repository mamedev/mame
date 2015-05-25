// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
#pragma once

#ifndef __NAMCO_H__
#define __NAMCO_H__

/* 8 voices max */
#define MAX_VOICES 8

#define MAX_VOLUME 16


#define MCFG_NAMCO_AUDIO_VOICES(_voices) \
	namco_audio_device::set_voices(*device, _voices);

#define MCFG_NAMCO_AUDIO_STEREO(_stereo) \
	namco_audio_device::set_stereo(*device, _stereo);


/* this structure defines the parameters for a channel */
struct sound_channel
{
	UINT32 frequency;
	UINT32 counter;
	INT32 volume[2];
	INT32 noise_sw;
	INT32 noise_state;
	INT32 noise_seed;
	UINT32 noise_counter;
	INT32 noise_hold;
	INT32 waveform_select;
};

class namco_audio_device : public device_t,
							public device_sound_interface
{
public:
	namco_audio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~namco_audio_device() {}

	// static configuration
	static void set_voices(device_t &device, int voices) { downcast<namco_audio_device &>(device).m_voices = voices; }
	static void set_stereo(device_t &device, int stereo) { downcast<namco_audio_device &>(device).m_stereo = stereo; }

protected:
	// device-level overrides
	virtual void device_start();

	// internal state

	void build_decoded_waveform( UINT8 *rgnbase );
	void update_namco_waveform(int offset, UINT8 data);
	UINT32 namco_update_one(stream_sample_t *buffer, int length, const INT16 *wave, UINT32 counter, UINT32 freq);

	/* data about the sound system */
	sound_channel m_channel_list[MAX_VOICES];
	sound_channel *m_last_channel;
	UINT8 *m_soundregs;
	UINT8 *m_wavedata;

	/* global sound parameters */
	int m_wave_size;
	INT32 m_sound_enable;
	sound_stream *m_stream;
	int m_namco_clock;
	int m_sample_rate;
	int m_f_fracbits;

	int m_voices;     /* number of voices */
	int m_stereo;     /* set to 1 to indicate stereo (e.g., System 1) */

	/* decoded waveform table */
	INT16 *m_waveform[MAX_VOLUME];

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

class namco_device : public namco_audio_device
{
public:
	namco_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( pacman_sound_enable_w );
	DECLARE_WRITE8_MEMBER( pacman_sound_w );

	void polepos_sound_enable(int enable);

	DECLARE_READ8_MEMBER( polepos_sound_r );
	DECLARE_WRITE8_MEMBER( polepos_sound_w );

protected:
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type NAMCO;

class namco_15xx_device : public namco_audio_device
{
public:
	namco_15xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( namco_15xx_w );
	DECLARE_READ8_MEMBER( sharedram_r );
	DECLARE_WRITE8_MEMBER( sharedram_w );

	void mappy_sound_enable(int enable);

protected:
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type NAMCO_15XX;

class namco_cus30_device : public namco_audio_device
{
public:
	namco_cus30_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( namcos1_cus30_w );   /* wavedata + sound registers + RAM */
	DECLARE_READ8_MEMBER( namcos1_cus30_r );
	DECLARE_WRITE8_MEMBER( namcos1_sound_w );

	DECLARE_WRITE8_MEMBER( pacman_sound_w );

protected:
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type NAMCO_CUS30;

#endif /* __NAMCO_H__ */
