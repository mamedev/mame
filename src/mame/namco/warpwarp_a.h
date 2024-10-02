// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
#ifndef MAME_NAMCO_WARPWARP_A_H
#define MAME_NAMCO_WARPWARP_A_H

#pragma once


class warpwarp_sound_device : public device_t, public device_sound_interface
{
public:
	warpwarp_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sound_w(u8 data);
	void music1_w(u8 data);
	void music2_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	TIMER_CALLBACK_MEMBER(sound_decay_tick);
	TIMER_CALLBACK_MEMBER(music_decay_tick);

private:
	// internal state
	std::unique_ptr<int16_t[]> m_decay;
	sound_stream *m_channel;
	u32 m_clock_16h = 0;
	u32 m_clock_1v = 0;
	int m_sound_latch;
	int m_music1_latch;
	int m_music2_latch;
	int m_sound_signal;
	int m_sound_volume;
	emu_timer   *m_sound_volume_timer;
	int m_music_signal;
	int m_music_volume;
	emu_timer   *m_music_volume_timer;
	int m_noise;

	int m_vcarry;
	int m_vcount;
	int m_mcarry;
	int m_mcount;
};

DECLARE_DEVICE_TYPE(WARPWARP_SOUND, warpwarp_sound_device)

#endif // MAME_NAMCO_WARPWARP_A_H
