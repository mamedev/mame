// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sanyo LC7580 LCD Driver

*/

#ifndef MAME_VIDEO_LC7580_H
#define MAME_VIDEO_LC7580_H

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


class lc7580_device : public device_t, public device_nvram_interface
{
public:
	lc7580_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto write_segs() { return m_write_segs.bind(); } // S pins, COM1/COM2 in offset

	void data_w(int state) { m_data = (state) ? 1 : 0; }
	void clk_w(int state);
	void ce_w(int state);
	void inh_w(int state) { m_blank = bool(state); refresh_output(); }

protected:
	lc7580_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override { refresh_output(); }

	// device_nvram_interface implementation
	virtual void nvram_default() override { ; }
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	void refresh_output();

	int m_data;
	int m_ce;
	int m_clk;
	bool m_blank;

	u64 m_shift;
	u64 m_latch[2];

	// callbacks
	devcb_write64 m_write_segs;
};

class lc7582_device : public lc7580_device
{
public:
	lc7582_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(LC7580, lc7580_device)
DECLARE_DEVICE_TYPE(LC7582, lc7582_device)

#endif // MAME_VIDEO_LC7580_H
