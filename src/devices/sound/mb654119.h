// license:GPL-2.0
// copyright-holders:Giulio Zausa
#ifndef MAME_SOUND_MB654119_H
#define MAME_SOUND_MB654119_H
#pragma once

#include <array>

class mb654119_device : public device_t
{
public:
	mb654119_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	void set_long(bool state) { m_pin_long = state; }
	void set_k64(bool state) { m_pin_k64 = state; }
	void write_rate(u8 data);
	void write_delay(u8 data);
	void write_depth(u8 data);
	void init_pulse();
	void process_frame(s16 input, s16 &left, s16 &right);
	void patch_input(u32 periods_back, s16 input);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static u16 rate_to_ticks(u8 rate);
	s32 read(u32 delay) const;
	void advance_lfo(u32 rising_offset);
	bool m_pin_long, m_pin_k64;
	// parameters
	u32 m_base_delay; // 1/256 sample units
	u32 m_depth;
	u32 m_ticks_per_half;
	u32 m_limit;
	u32 m_delay_step; // samples per DELAY count

	// LFO
	u32 m_acc;   // phase, 1/4096 sample units
	u32 m_peak;  // phase at the previous turnaround
	u32 m_ramp;  // free running sample counter
	u32 m_ticks; // reset at each turnaround
	bool m_dir;  // which channel is currently rising
	bool m_par;  // slope accumulator ticks every 2nd sample

	// delay line
	std::array<s16, 16384> m_buf;
	u32 m_w;
	u32 m_mask;
};
DECLARE_DEVICE_TYPE(MB654119, mb654119_device)
#endif // MAME_SOUND_MB654119_H
