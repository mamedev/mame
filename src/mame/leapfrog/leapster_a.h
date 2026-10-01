// license:BSD-3-Clause
// copyright-holders:Alice Shelton
#ifndef MAME_LEAPFROG_LEAPSTER_A_H
#define MAME_LEAPFROG_LEAPSTER_A_H

#pragma once

class leapster_snd_device : public device_t,
							public device_sound_interface
{
public:
	leapster_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void map(address_map &map);

	void do_voice_command(uint8_t command, uint8_t voice);

	uint8_t get_triggered_voices()
	{
		m_stream->update();

		// Voice 0 doesn't have any special significance so I'm not sure why it's out of place here
		//   Maybe historical reasons from older revisions of the audio hardware?
		uint8_t triggered_voices = ((uint8_t) m_channel_triggered[0]) << 7;

		for(int i = 1; i < 8; i++)
		{
			triggered_voices |= ((uint8_t) m_channel_triggered[i]) << (i - 1);
		}

		return triggered_voices;
	}

	void set_address_space(address_space *space)
	{
		m_space = space;
	}

private:
	virtual void device_start() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

	void celp_codebook_w(uint32_t off, uint16_t value);

	void voice_generic_w(uint32_t off, uint16_t value);
	void voice_start_w(uint32_t off, uint16_t value);
	void voice_end_w(uint32_t off, uint16_t value);
	void voice_volume_w(uint32_t off, uint16_t value);
	void voice_pitch_w(uint32_t off, uint16_t value);

	static int16_t conv_alaw_sample(uint8_t raw_sample)
	{
		raw_sample ^= 0x55;

		auto exp = BIT(raw_sample, 4, 3);
		uint16_t base = (BIT(raw_sample, 0, 4) << 1) | 1;

		if(exp != 0)
		{
			base |= 0x20;
			exp -= 1;
		}

		int16_t sample = base << (exp + 2);

		if(BIT(raw_sample, 7))
		{
			sample = -sample;
		}

		return sample;
	}

	sound_stream *m_stream;
	address_space *m_space;

	uint32_t m_celp_codebook;

	uint32_t m_data_source_start[8]{};
	uint32_t m_data_source_end[8]{};
	uint16_t m_volume[8]{};
	uint16_t m_pitch[5]{};

	bool m_channel_triggered[8]{};
	float m_channel_dx[8]{};
	float m_channel_index[8]{};
};

DECLARE_DEVICE_TYPE(LEAPSTER_SOUND, leapster_snd_device)

#endif
