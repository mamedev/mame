// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
Hitachi HD61602 LCD Driver.
51 segment outputs, 4 common outputs, configurable duty cycle.

Pinout (80-pin QFP)

pin     desc
------------------------------
1     = VDD
2     = READY
3     = _CS
4     = _WE
5     = _RE
6     = SB
7-10  = D7-D4
11    = VSS
12-15 = D3-D0
16    = VREF1
17    = VREF2
18,19 = VC2
20-22 = V1-V3
23-26 = COM0-COM3
27-77 = SEG50-SEG0
78    = SYNC
79,80 = OSC2,OSC1
*/

#ifndef MAME_VIDEO_HD61602_H
#define MAME_VIDEO_HD61602_H

#pragma once

class hd61602_device : public device_t
{
public:
	static constexpr const int NCOM = 4;   // COM (common) outputs, aka rows.
	static constexpr const int NSEG = 51;  // SEG (segment) outputs, aka columns.

	hd61602_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	auto write_segs() { return m_write_segs.bind(); }

	void data_w(u8 data);

	// Byte counter resets when /CS and /RE are both driven low.
	void reset_counter_strobe();

protected:
	virtual void device_start() override ATTR_COLD;

private:
	void set_ram_bit(u8 com, u8 seg, u8 bit);
	void set_drive_mode(u8 drive_mode);
	TIMER_CALLBACK_MEMBER(refresh_timer_tick);

	enum display_mode
	{
		DM_STATIC = 0,
		DM_HALF,     // 1/2 duty cycle.
		DM_THIRD,    // 1/3 duty cycle.
		DM_QUARTER,  // 1/4 duty cycle.
	};

	emu_timer *m_refresh_timer;
	devcb_write64 m_write_segs;

	u8 m_count;
	u16 m_data;
	std::array<u64, NCOM> m_ram;
	bool m_blank;
	u8 m_display_mode;
	u8 m_drive_mode;
	u8 m_com_count;
	u8 m_active_com;
};

DECLARE_DEVICE_TYPE(HD61602, hd61602_device)

#endif // MAME_VIDEO_HD61602_H
