// license: BSD-3-Clause
// copyright-holders: Devin Acker

#ifndef MAME_SOUND_CF61909_H
#define MAME_SOUND_CF61909_H

#pragma once

#include "machine/clock.h"
#include "dirom.h"

#include <array>


class cf61909_device : public device_t, public device_sound_interface, public device_rom_interface<18>
{
public:
	cf61909_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	int sync_r() { return m_sample_clock->signal_r(); }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;
	virtual void rom_bank_pre_change() override;

private:
	/*
	* Jaminator patent specifies 11.127 kHz sample rate, but the real thing sounds like it has 4x
	* oversampling. The sync clock output (m_sample_clock) seems to fluctuate quite a bit, but
	* 11.127 kHz is pretty close to average.
	*/
	static constexpr unsigned CLOCKS_PER_SAMPLE = 247; // based on 11 MHz clock

	struct voice_t
	{
		u8 m_regs[16] = {0};
		u32 m_start = 0;
		u16 m_loop = 0;
		u16 m_pos = 0;
		u16 m_pitch = 0;
		u16 m_pitch_counter = 0;
		u8 m_volume = 0;
	};
	std::array<voice_t, 8> m_voice;

	u32 m_data_offset;

	required_device<clock_device> m_sample_clock;
	sound_stream *m_stream;
};

DECLARE_DEVICE_TYPE(CF61909, cf61909_device)

#endif // MAME_SOUND_CF61909_H
