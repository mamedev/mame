// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
#ifndef MAME_AUDIO_WIPING_H
#define MAME_AUDIO_WIPING_H

#pragma once

class wiping_sound_device : public device_t, public device_sound_interface
{
public:
	wiping_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( sound_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	/* 8 voices max */
	static constexpr unsigned MAX_VOICES = 8;

	/* this structure defines the parameters for a channel */
	struct wp_sound_channel
	{
		int frequency;
		int counter;
		int volume;
		const uint8_t *wave;
		int oneshot;
		int oneshotplaying;
	};

	// internal state

	/* data about the sound system */
	wp_sound_channel m_channel_list[MAX_VOICES];
	wp_sound_channel *m_last_channel;

	/* global sound parameters */
	const uint8_t *m_sound_prom;
	const uint8_t *m_sound_rom;
	int m_num_voices;
	int m_sound_enable;
	sound_stream *m_stream;

	/* mixer tables and internal buffers */
	std::unique_ptr<int16_t[]> m_mixer_table;
	int16_t *m_mixer_lookup;
	std::unique_ptr<short[]> m_mixer_buffer;
	std::unique_ptr<short[]> m_mixer_buffer_2;

	uint8_t m_soundregs[0x4000];

	void make_mixer_table(int voices, int gain);
};

DECLARE_DEVICE_TYPE(WIPING_CUSTOM, wiping_sound_device)

#endif // MAME_AUDIO_WIPING_H
