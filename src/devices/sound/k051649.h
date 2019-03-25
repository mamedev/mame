// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_SOUND_K051649_H
#define MAME_SOUND_K051649_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k051649_device

class k051649_device : public device_t,
						public device_sound_interface
{
public:
	k051649_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( k051649_waveform_w );
	DECLARE_READ8_MEMBER ( k051649_waveform_r );
	DECLARE_WRITE8_MEMBER( k051649_volume_w );
	DECLARE_WRITE8_MEMBER( k051649_frequency_w );
	DECLARE_WRITE8_MEMBER( k051649_keyonoff_w );
	DECLARE_WRITE8_MEMBER( k051649_test_w );
	DECLARE_READ8_MEMBER ( k051649_test_r );

	DECLARE_WRITE8_MEMBER( k052539_waveform_w );
	DECLARE_READ8_MEMBER ( k052539_waveform_r );

	void scc_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// Parameters for a channel
	struct sound_channel
	{
		sound_channel() :
			counter(0),
			frequency(0),
			volume(0),
			key(0)
		{
			std::fill(std::begin(waveram), std::end(waveram), 0);
		}

		unsigned long counter;
		int frequency;
		int volume;
		int key;
		signed char waveram[32];
	};

	void make_mixer_table(int voices);

	sound_channel m_channel_list[5];

	/* global sound parameters */
	sound_stream *m_stream;
	int m_mclock;
	int m_rate;

	/* mixer tables and internal buffers */
	std::unique_ptr<int16_t[]> m_mixer_table;
	int16_t *m_mixer_lookup;
	std::vector<short> m_mixer_buffer;

	/* chip registers */
	uint8_t m_test;
};

DECLARE_DEVICE_TYPE(K051649, k051649_device)

#endif // MAME_SOUND_K051649_H
