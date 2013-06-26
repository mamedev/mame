/**********************************************************************

    RCA CDP1871 Keyboard Encoder emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                    D1   1 |*    \_/     | 40  Vdd
                    D2   2 |             | 39  SHIFT
                    D3   3 |             | 38  CONTROL
                    D4   4 |             | 37  ALPHA
                    D5   5 |             | 36  DEBOUNCE
                    D6   6 |             | 35  _RTP
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
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1871_ADD(_tag, _intrf, _clock) \
	MCFG_DEVICE_ADD(_tag, CDP1871, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

#define CDP1871_INTERFACE(name) \
	const cdp1871_interface (name)=



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1871_interface

struct cdp1871_interface
{
	devcb_read8         in_d1_cb;
	devcb_read8         in_d2_cb;
	devcb_read8         in_d3_cb;
	devcb_read8         in_d4_cb;
	devcb_read8         in_d5_cb;
	devcb_read8         in_d6_cb;
	devcb_read8         in_d7_cb;
	devcb_read8         in_d8_cb;
	devcb_read8         in_d9_cb;
	devcb_read8         in_d10_cb;
	devcb_read8         in_d11_cb;

	devcb_read_line     in_shift_cb;
	devcb_read_line     in_control_cb;
	devcb_read_line     in_alpha_cb;

	// this gets called for every change of the DA pin (pin 33)
	devcb_write_line    out_da_cb;

	// this gets called for every change of the RPT pin (pin 35)
	devcb_write_line    out_rpt_cb;
};


// ======================> cdp1871_device

class cdp1871_device :  public device_t,
						public cdp1871_interface
{
public:
	// construction/destruction
	cdp1871_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( data_r );

	DECLARE_READ_LINE_MEMBER( da_r );
	DECLARE_READ_LINE_MEMBER( rpt_r );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void change_output_lines();
	void clock_scan_counters();
	void detect_keypress();

private:
	devcb_resolved_write_line       m_out_da_func;
	devcb_resolved_write_line       m_out_rpt_func;
	devcb_resolved_read8            m_in_d_func[11];
	devcb_resolved_read_line        m_in_shift_func;
	devcb_resolved_read_line        m_in_control_func;
	devcb_resolved_read_line        m_in_alpha_func;

	bool m_inhibit;                 // scan counter clock inhibit
	int m_sense;                    // sense input scan counter
	int m_drive;                    // modifier inputs

	int m_shift;                    // latched shift modifier
	int m_control;                  // latched control modifier

	int m_da;                       // data available flag
	int m_next_da;                  // next value of data available flag
	int m_rpt;                      // repeat flag
	int m_next_rpt;                 // next value of repeat flag

	// timers
	emu_timer *m_scan_timer;        // keyboard scan timer
};


// device type definition
extern const device_type CDP1871;



#endif
