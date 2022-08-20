// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sanyo LC7582 LCD Driver

*/

#ifndef MAME_VIDEO_LC7582_H
#define MAME_VIDEO_LC7582_H

#pragma once

/*

quick pinout reference (64-pin QFP)

pin     desc
------------------------------
1-54  = S1-S53 segment outputs, pin 24 N/C, pins 45-54 also used with AD/DSP
55    = OSC
56    = Vdd
57    = _INH
58    = Vlcd
59    = Vss
60    = CE   \
61    = CLK  | serial input
62    = DATA /
63,64 = COM1/COM2 outputs

*/


class lc7582_device : public device_t
{
public:
	lc7582_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto write_segs() { return m_write_segs.bind(); } // S pins, COM1/COM2 in offset

	DECLARE_WRITE_LINE_MEMBER(data_w) { m_data = (state) ? 1 : 0; }
	DECLARE_WRITE_LINE_MEMBER(clk_w);
	DECLARE_WRITE_LINE_MEMBER(ce_w);
	DECLARE_WRITE_LINE_MEMBER(inh_w) { m_blank = bool(state); refresh_output(); }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	void refresh_output();

	int m_data;
	int m_ce;
	int m_clk;
	bool m_blank;

	int m_duty;
	int m_addsp;
	u64 m_shift;
	u64 m_latch[2];

	// callbacks
	devcb_write64 m_write_segs;
};


DECLARE_DEVICE_TYPE(LC7582, lc7582_device)

#endif // MAME_VIDEO_LC7582_H
