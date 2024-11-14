// license:BSD-3-Clause
// copyright-holders:hap
/*

  Texas Instruments TMC0999 256x4 RAM

*/

#ifndef MAME_MACHINE_TMC0999_H
#define MAME_MACHINE_TMC0999_H

#pragma once

/*

quick pinout reference (18-pin DIP)

17,18,1,2: data outputs (do_r)
3-6: data inputs (di_w)
7: address latch strobe (adr_w)
13: data input strobe (wr_w)
14: data output enable (rd_w)

*/


class tmc0999_device : public device_t
{
public:
	tmc0999_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void di_w(u8 data);
	u8 do_r();
	void wr_w(int state);
	void rd_w(int state);
	void adr_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// input pins state
	u8 m_data;
	int m_wr;
	int m_rd;
	int m_adr_strobe;

	// internal state
	u8 m_ram_address;
	u8 m_ram[0x100];
};


DECLARE_DEVICE_TYPE(TMC0999, tmc0999_device)

#endif // MAME_MACHINE_TMC0999_H
