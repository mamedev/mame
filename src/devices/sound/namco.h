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
	uint32_t frequency;
	uint32_t counter;
	int32_t volume[2];
	int32_t noise_sw;
	int32_t noise_state;
	int32_t noise_seed;
	uint32_t noise_counter;
	int32_t noise_hold;
	int32_t waveform_select;
};

class namco_audio_device : public device_t,
							public device_sound_interface
{
public:
	namco_audio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	~namco_audio_device() {}

	// static configuration
	static void set_voices(device_t &device, int voices) { downcast<namco_audio_device &>(device).m_voices = voices; }
	static void set_stereo(device_t &device, int stereo) { downcast<namco_audio_device &>(device).m_stereo = stereo; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// internal state

	void build_decoded_waveform( uint8_t *rgnbase );
	void update_namco_waveform(int offset, uint8_t data);
	uint32_t namco_update_one(stream_sample_t *buffer, int length, const int16_t *wave, uint32_t counter, uint32_t freq);

	/* waveform region */
	optional_region_ptr<uint8_t> m_wave_ptr;

	/* data about the sound system */
	sound_channel m_channel_list[MAX_VOICES];
	sound_channel *m_last_channel;
	uint8_t *m_soundregs;
	uint8_t *m_wavedata;

	/* global sound parameters */
	int m_wave_size;
	int32_t m_sound_enable;
	sound_stream *m_stream;
	int m_namco_clock;
	int m_sample_rate;
	int m_f_fracbits;

	int m_voices;     /* number of voices */
	int m_stereo;     /* set to 1 to indicate stereo (e.g., System 1) */

	/* decoded waveform table */
	int16_t *m_waveform[MAX_VOLUME];

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};

class namco_device : public namco_audio_device
{
public:
	namco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void pacman_sound_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pacman_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void polepos_sound_enable(int enable);

	uint8_t polepos_sound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void polepos_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};

extern const device_type NAMCO;

class namco_15xx_device : public namco_audio_device
{
public:
	namco_15xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void namco_15xx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sharedram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sharedram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void mappy_sound_enable(int enable);

protected:
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};

extern const device_type NAMCO_15XX;

class namco_cus30_device : public namco_audio_device
{
public:
	namco_cus30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void namcos1_cus30_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);   /* wavedata + sound registers + RAM */
	uint8_t namcos1_cus30_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void namcos1_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void pacman_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};

extern const device_type NAMCO_CUS30;

#endif /* __NAMCO_H__ */
