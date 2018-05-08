// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Dan Boris
/*************************************************************************

    rokola hardware

*************************************************************************/
#ifndef MAME_AUDIO_SNK6502_H
#define MAME_AUDIO_SNK6502_H

#pragma once

#include "sound/discrete.h"
#include "sound/samples.h"


class snk6502_sound_device : public device_t, public device_sound_interface
{
public:
	snk6502_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( sasuke_sound_w );
	DECLARE_WRITE8_MEMBER( satansat_sound_w );
	DECLARE_WRITE8_MEMBER( vanguard_sound_w );
	DECLARE_WRITE8_MEMBER( vanguard_speech_w );
	DECLARE_WRITE8_MEMBER( fantasy_sound_w );
	DECLARE_WRITE8_MEMBER( fantasy_speech_w );

	void set_music_clock(double clock_time);
	void set_music_freq(int freq);
	int music0_playing();

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	static constexpr unsigned CHANNELS = 3;

	struct TONE
	{
		int mute;
		int offset;
		int base;
		int mask;
		int32_t   sample_rate;
		int32_t   sample_step;
		int32_t   sample_cur;
		int16_t   form[16];
	};

	// internal state
	TONE m_tone_channels[CHANNELS];
	int32_t m_tone_clock_expire;
	int32_t m_tone_clock;
	sound_stream * m_tone_stream;

	optional_device<samples_device> m_samples;
	uint8_t *m_ROM;
	int m_Sound0StopOnRollover;
	uint8_t m_LastPort1;

	int m_hd38880_cmd;
	uint32_t m_hd38880_addr;
	int m_hd38880_data_bytes;
	double m_hd38880_speed;

	inline void validate_tone_channel(int channel);
	void sasuke_build_waveform(int mask);
	void satansat_build_waveform(int mask);
	void build_waveform(int channel, int mask);
	void speech_w(uint8_t data, const uint16_t *table, int start);
};

DECLARE_DEVICE_TYPE(SNK6502, snk6502_sound_device)

DISCRETE_SOUND_EXTERN( fantasy_discrete );

extern char const *const sasuke_sample_names[];
extern char const *const vanguard_sample_names[];
extern char const *const fantasy_sample_names[];

#endif // MAME_AUDIO_SNK6502_H
