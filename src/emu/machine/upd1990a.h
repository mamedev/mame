/**********************************************************************

    NEC uPD1990AC Serial I/O Calendar & Clock emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                    C2   1 |*    \_/     | 14  Vdd
                    C1   2 |             | 13  XTAL
                    C0   3 |             | 12  _XTAL
                   STB   4 |  uPD1990AC  | 11  OUT ENBL
                    CS   5 |             | 10  TP
               DATA IN   6 |             | 9   DATA OUT
                   GND   7 |_____________| 8   CLK

**********************************************************************/

#pragma once

#ifndef __UPD1990A__
#define __UPD1990A__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************




//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UPD1990A_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD((_tag), UPD1990A, _clock)	\
	MCFG_DEVICE_CONFIG(_config)

#define UPD1990A_INTERFACE(name) \
	const upd1990a_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd1990a_interface

struct upd1990a_interface
{
	devcb_write_line		m_out_data_cb;
	devcb_write_line		m_out_tp_cb;
};


// ======================> upd1990a_device

class upd1990a_device :	public device_t,
						public device_rtc_interface,
                        public upd1990a_interface
{
public:
    // construction/destruction
    upd1990a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( oe_w );
	DECLARE_WRITE_LINE_MEMBER( cs_w );
	DECLARE_WRITE_LINE_MEMBER( stb_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_WRITE_LINE_MEMBER( c0_w );
	DECLARE_WRITE_LINE_MEMBER( c1_w );
	DECLARE_WRITE_LINE_MEMBER( c2_w );
	DECLARE_WRITE_LINE_MEMBER( data_in_w );
	DECLARE_READ_LINE_MEMBER( data_out_r );
	DECLARE_READ_LINE_MEMBER( tp_r );

protected:
    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();
    virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second);

private:
	static const device_timer_id TIMER_CLOCK = 0;
	static const device_timer_id TIMER_TP = 1;
	static const device_timer_id TIMER_DATA_OUT = 2;
	static const device_timer_id TIMER_TEST_MODE = 3;

	devcb_resolved_write_line	m_out_data_func;
	devcb_resolved_write_line	m_out_tp_func;

	UINT8 m_time_counter[5];	// time counter
	UINT8 m_shift_reg[5];		// shift register

	int m_oe;					// output enable
	int m_cs;					// chip select
	int m_stb;					// strobe
	int m_data_in;				// data in
	int m_data_out;				// data out
	int m_c;					// command
	int m_clk;					// shift clock
	int m_tp;					// time pulse
    int m_c_unlatched;          // command waiting for STB

	// timers
	emu_timer *m_timer_clock;
	emu_timer *m_timer_tp;
	emu_timer *m_timer_data_out;
	emu_timer *m_timer_test_mode;
};


// device type definition
extern const device_type UPD1990A;



#endif
