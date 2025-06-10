// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HD61603 LCD Driver

*/

#ifndef MAME_VIDEO_HD61603_H
#define MAME_VIDEO_HD61603_H

#pragma once

/*

quick pinout reference (80-pin QFP)

pin     desc
------------------------------
1     = VDD
2     = READY
3     = _CS
4     = _WE
5     = _RE
6     = SB
7-10  = D3-D0 (data_w)
11    = VSS
12    = V3
13    = COM0
14-77 = SEG63-SEG0: LCD segment outputs
78    = SYNC
79,80 = OSC2,OSC1

*/


class hd61603_device : public device_t
{
public:
	hd61603_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto write_segs() { return m_write_segs.bind(); }

	void data_w(u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_count;
	u16 m_data;
	u64 m_ram;
	int m_blank;

	// callbacks
	devcb_write64 m_write_segs;
};


DECLARE_DEVICE_TYPE(HD61603, hd61603_device)

#endif // MAME_VIDEO_HD61603_H
