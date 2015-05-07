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

#pragma once

#ifndef __CDP1871__
#define __CDP1871__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1871_D1_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d1_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D2_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d2_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D3_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d3_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D4_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d4_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D5_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d5_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D6_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d6_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D7_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d7_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D8_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d8_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D9_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d9_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D10_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d10_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_D11_CALLBACK(_read) \
	devcb = &cdp1871_device::set_d11_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1871_DA_CALLBACK(_write) \
	devcb = &cdp1871_device::set_da_wr_callback(*device, DEVCB_##_write);

#define MCFG_CDP1871_RPT_CALLBACK(_write) \
	devcb = &cdp1871_device::set_rpt_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1871_device

class cdp1871_device :  public device_t
{
public:
	// construction/destruction
	cdp1871_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_d1_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d1.set_callback(object); }
	template<class _Object> static devcb_base &set_d2_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d2.set_callback(object); }
	template<class _Object> static devcb_base &set_d3_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d3.set_callback(object); }
	template<class _Object> static devcb_base &set_d4_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d4.set_callback(object); }
	template<class _Object> static devcb_base &set_d5_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d5.set_callback(object); }
	template<class _Object> static devcb_base &set_d6_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d6.set_callback(object); }
	template<class _Object> static devcb_base &set_d7_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d7.set_callback(object); }
	template<class _Object> static devcb_base &set_d8_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d8.set_callback(object); }
	template<class _Object> static devcb_base &set_d9_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d9.set_callback(object); }
	template<class _Object> static devcb_base &set_d10_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d10.set_callback(object); }
	template<class _Object> static devcb_base &set_d11_rd_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_read_d11.set_callback(object); }
	template<class _Object> static devcb_base &set_da_wr_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_write_da.set_callback(object); }
	template<class _Object> static devcb_base &set_rpt_wr_callback(device_t &device, _Object object) { return downcast<cdp1871_device &>(device).m_write_rpt.set_callback(object); }

	DECLARE_READ8_MEMBER( read );

	DECLARE_READ_LINE_MEMBER( da_r ) { return m_da; }
	DECLARE_READ_LINE_MEMBER( rpt_r ) { return m_rpt; }

	DECLARE_WRITE_LINE_MEMBER( shift_w ) { m_shift = state; }
	DECLARE_WRITE_LINE_MEMBER( control_w ) { m_control = state; }
	DECLARE_WRITE_LINE_MEMBER( alpha_w ) { m_alpha = state; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void change_output_lines();
	void clock_scan_counters();
	void detect_keypress();

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

	static const UINT8 key_codes[4][11][8];
};


// device type definition
extern const device_type CDP1871;



#endif
