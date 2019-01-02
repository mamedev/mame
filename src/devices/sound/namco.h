// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
#ifndef MAME_SOUND_NAMCO_H
#define MAME_SOUND_NAMCO_H

#pragma once


class namco_audio_device : public device_t,
							public device_sound_interface
{
public:
	// configuration
	void set_voices(int voices) { m_voices = voices; }
	void set_stereo(bool stereo) { m_stereo = stereo; }

	DECLARE_WRITE_LINE_MEMBER(sound_enable_w);

protected:
	static constexpr unsigned MAX_VOICES = 8;
	static constexpr unsigned MAX_VOLUME = 16;

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

	namco_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;

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
	bool m_sound_enable;
	sound_stream *m_stream;
	int m_namco_clock;
	int m_sample_rate;
	int m_f_fracbits;

	int m_voices;     /* number of voices */
	bool m_stereo;    /* set to indicate stereo (e.g., System 1) */

	/* decoded waveform table */
	int16_t *m_waveform[MAX_VOLUME];

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};

class namco_device : public namco_audio_device
{
public:
	namco_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER(pacman_sound_w);

	void polepos_sound_enable(int enable);

	DECLARE_READ8_MEMBER(polepos_sound_r);
	DECLARE_WRITE8_MEMBER(polepos_sound_w);

protected:
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};


class namco_15xx_device : public namco_audio_device
{
public:
	namco_15xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( namco_15xx_w );
	DECLARE_READ8_MEMBER( sharedram_r );
	DECLARE_WRITE8_MEMBER( sharedram_w );

	DECLARE_WRITE_LINE_MEMBER(mappy_sound_enable);

protected:
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};


class namco_cus30_device : public namco_audio_device
{
public:
	namco_cus30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( namcos1_cus30_w );   /* wavedata + sound registers + RAM */
	DECLARE_READ8_MEMBER( namcos1_cus30_r );
	DECLARE_WRITE8_MEMBER( namcos1_sound_w );

	DECLARE_WRITE8_MEMBER( pacman_sound_w );

protected:
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};


DECLARE_DEVICE_TYPE(NAMCO,       namco_device)
DECLARE_DEVICE_TYPE(NAMCO_15XX,  namco_15xx_device)
DECLARE_DEVICE_TYPE(NAMCO_CUS30, namco_cus30_device)

#endif // MAME_SOUND_NAMCO_H
