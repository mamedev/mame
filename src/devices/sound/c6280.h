// license:BSD-3-Clause
// copyright-holders:Charles MacDonald
#ifndef MAME_SOUND_C6280_H
#define MAME_SOUND_C6280_H

#pragma once

class c6280_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // Incorrect / Not verified noise / LFO output

	c6280_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// write only
	void c6280_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	struct channel {
		u16 frequency;
		u8 control;
		u8 balance;
		u8 waveform[32];
		u8 index;
		s16 dda;
		u8 noise_control;
		s32 noise_counter;
		u32 noise_frequency;
		u32 noise_seed;
		s32 tick;
	};

	// internal state
	sound_stream *m_stream;
	u8 m_select;
	u8 m_balance;
	u8 m_lfo_frequency;
	u8 m_lfo_control;
	channel m_channel[8];
	s16 m_volume_table[32];
};

DECLARE_DEVICE_TYPE(C6280, c6280_device)

#endif // MAME_SOUND_C6280_H
