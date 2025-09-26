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
	k051649_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void k051649_waveform_w(offs_t offset, u8 data);
	u8 k051649_waveform_r(offs_t offset);
	void k051649_volume_w(offs_t offset, u8 data);
	void k051649_frequency_w(offs_t offset, u8 data);
	void k051649_keyonoff_w(u8 data);
	void k051649_test_w(u8 data);
	u8 k051649_test_r(address_space &space);

	void k052539_waveform_w(offs_t offset, u8 data);
	u8 k052539_waveform_r(offs_t offset);

	void scc_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// parameters for a channel
	struct sound_channel
	{
		sound_channel() :
			counter(0),
			clock(0),
			frequency(0),
			volume(0),
			sample(0),
			key(false)
		{
			std::fill(std::begin(waveram), std::end(waveram), 0);
		}

		u8 counter;     // address counter for wavetable
		u16 clock;      // internal clock
		u16 frequency;  // frequency; result: (input clock / (32 * (frequency + 1)))
		u8 volume;      // volume
		s16 sample;     // latched sample data
		bool key;       // keyon/off
		s8 waveram[32]; // 32 byte wavetable
	};

	sound_channel m_channel_list[5];

	sound_stream *m_stream;
	u8 m_test; // test register
};

DECLARE_DEVICE_TYPE(K051649, k051649_device)

#endif // MAME_SOUND_K051649_H
