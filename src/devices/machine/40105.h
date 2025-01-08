// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CD40105/HC40105 4-bit x 16-word FIFO Register

***********************************************************************
                                ____   ____
                       /OE   1 |*   \_/    | 16  Vcc
                       DIR   2 |           | 15  /SO
                        SI   3 |           | 14  DOR
                        D0   4 |           | 13  Q0
                        D1   5 |   40105   | 12  Q1
                        D2   6 |           | 11  Q2
                        D3   7 |           | 10  Q3
                       GND   8 |___________|  9  MR

**********************************************************************/

#ifndef MAME_MACHINE_40105_H
#define MAME_MACHINE_40105_H

#pragma once

#include <queue>


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> cmos_40105_device

class cmos_40105_device :  public device_t
{
public:
	// construction/destruction
	cmos_40105_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto in_ready_cb() { return m_write_dir.bind(); }
	auto out_ready_cb() { return m_write_dor.bind(); }
	auto out_cb() { return m_write_q.bind(); }

	u8 read();
	void write(u8 data);

	void si_w(int state);
	void so_w(int state);

	int dir_r();
	int dor_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// private helpers
	void load_input();
	void output_ready();

	// callbacks
	devcb_write_line m_write_dir;
	devcb_write_line m_write_dor;
	devcb_write8 m_write_q;

	std::queue<u8> m_fifo;

	u8 m_d;
	u8 m_q;

	bool m_dir;
	bool m_dor;
	bool m_si;
	bool m_so;
};


// device type definition
DECLARE_DEVICE_TYPE(CD40105, cmos_40105_device)

#endif // MAME_MACHINE_40105_H
