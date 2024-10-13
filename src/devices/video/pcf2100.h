// license:BSD-3-Clause
// copyright-holders:hap
/*

  Philips PCF2100 family LCD Driver

*/

#ifndef MAME_VIDEO_PCF2100_H
#define MAME_VIDEO_PCF2100_H

#pragma once

/*

pinout reference (brief)
see datasheet for details, pin numbers differ per chip

inputs:

OSC = oscillator input
CLB = clock burst
DATA = data line
DLEN = data line enable

outputs:

S: LCD driver outputs
BP1/BP2: backplane drivers (LCD commons)

*/

class pcf2100_device : public device_t
{
public:
	pcf2100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto write_segs() { return m_write_segs.bind(); }

	void clb_w(int state);
	void data_w(int state) { m_data = (state) ? 1 : 0; }
	void dlen_w(int state) { m_dlen = (state) ? 1 : 0; }

protected:
	pcf2100_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 bpmax, u8 smax);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_output_latches);

	emu_timer *m_lcd_timer;

	const u8 m_bpmax; // number of BP pins
	const u8 m_smax; // number of S pins
	u64 m_shift;
	u8 m_count;
	u8 m_bpout;
	u32 m_latch[2];

	// pin state
	int m_clb;
	int m_data;
	int m_dlen;

	// callbacks
	devcb_write32 m_write_segs;
};

class pcf2110_device : public pcf2100_device
{
public:
	pcf2110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class pcf2111_device : public pcf2100_device
{
public:
	pcf2111_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class pcf2112_device : public pcf2100_device
{
public:
	pcf2112_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(PCF2100, pcf2100_device)
DECLARE_DEVICE_TYPE(PCF2110, pcf2110_device)
DECLARE_DEVICE_TYPE(PCF2111, pcf2111_device)
DECLARE_DEVICE_TYPE(PCF2112, pcf2112_device)

#endif // MAME_VIDEO_PCF2100_H
