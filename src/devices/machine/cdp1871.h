// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1871 Keyboard Encoder emulation

**********************************************************************
                            _____   _____
                    D1   1 |*    \_/     | 40  Vdd
                    D2   2 |             | 39  SHIFT
                    D3   3 |             | 38  CONTROL
                    D4   4 |             | 37  ALPHA
                    D5   5 |             | 36  DEBOUNCE
                    D6   6 |             | 35  _RPT
                    D7   7 |             | 34  TPB
                    D8   8 |             | 33  _DA
                    D9   9 |             | 32  BUS 7
                   D10  10 |   CDP1871   | 31  BUS 6
                   D11  11 |             | 30  BUS 5
                    S1  12 |             | 29  BUS 4
                    S2  13 |             | 28  BUS 3
                    S3  14 |             | 27  BUS 2
                    S4  15 |             | 26  BUS 1
                    S5  16 |             | 25  BUS 0
                    S6  17 |             | 24  CS4
                    S7  18 |             | 23  CS3
                    S8  19 |             | 22  CS2
                   Vss  20 |_____________| 21  _CS1

**********************************************************************/

#ifndef MAME_MACHINE_CDP1871_H
#define MAME_MACHINE_CDP1871_H

#pragma once

class cdp1871_device :  public device_t
{
public:
	// construction/destruction
	cdp1871_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto d1_callback() { return m_read_d1.bind(); }
	auto d2_callback() { return m_read_d2.bind(); }
	auto d3_callback() { return m_read_d3.bind(); }
	auto d4_callback() { return m_read_d4.bind(); }
	auto d5_callback() { return m_read_d5.bind(); }
	auto d6_callback() { return m_read_d6.bind(); }
	auto d7_callback() { return m_read_d7.bind(); }
	auto d8_callback() { return m_read_d8.bind(); }
	auto d9_callback() { return m_read_d9.bind(); }
	auto d10_callback() { return m_read_d10.bind(); }
	auto d11_callback() { return m_read_d11.bind(); }
	auto da_callback() { return m_write_da.bind(); }
	auto rpt_callback() { return m_write_rpt.bind(); }

	uint8_t read();

	int da_r() { return m_da; }
	int rpt_r() { return m_rpt; }

	void shift_w(int state) { m_shift = state; }
	void control_w(int state) { m_control = state; }
	void alpha_w(int state) { m_alpha = state; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	void change_output_lines();
	void clock_scan_counters();
	void detect_keypress();

	TIMER_CALLBACK_MEMBER(perform_scan);

private:
	devcb_read8            m_read_d1;
	devcb_read8            m_read_d2;
	devcb_read8            m_read_d3;
	devcb_read8            m_read_d4;
	devcb_read8            m_read_d5;
	devcb_read8            m_read_d6;
	devcb_read8            m_read_d7;
	devcb_read8            m_read_d8;
	devcb_read8            m_read_d9;
	devcb_read8            m_read_d10;
	devcb_read8            m_read_d11;
	devcb_write_line       m_write_da;
	devcb_write_line       m_write_rpt;

	bool m_inhibit;                 // scan counter clock inhibit
	int m_sense;                    // sense input scan counter
	int m_drive;                    // modifier inputs

	int m_shift;
	int m_shift_latch;              // latched shift modifier
	int m_control;
	int m_control_latch;            // latched control modifier
	int m_alpha;

	int m_da;                       // data available flag
	int m_next_da;                  // next value of data available flag
	int m_rpt;                      // repeat flag
	int m_next_rpt;                 // next value of repeat flag

	// timers
	emu_timer *m_scan_timer;        // keyboard scan timer

	static const uint8_t key_codes[4][11][8];
};

DECLARE_DEVICE_TYPE(CDP1871, cdp1871_device)

#endif // MAME_MACHINE_CDP1871_H
