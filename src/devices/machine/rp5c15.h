// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Ricoh RP5C15 Real Time Clock emulation

**********************************************************************
                            _____   _____
                   _CS   1 |*    \_/     | 18  Vcc
                    CS   2 |             | 17  OSCOUT
                CLKOUT   3 |             | 16  OSCIN
                    A0   4 |   RP5C15    | 15  _ALARM
                    A1   5 |   RF5C15    | 14  D3
                    A2   6 |   RJ5C15    | 13  D2
                    A3   7 |             | 12  D1
                   _RD   8 |             | 11  D0
                   GND   9 |_____________| 10  _WR

**********************************************************************/

#pragma once

#ifndef __RP5C15__
#define __RP5C15__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RP5C15_OUT_ALARM_CB(_devcb) \
	devcb = &rp5c15_device::set_out_alarm_callback(*device, DEVCB_##_devcb);

#define MCFG_RP5C15_OUT_CLKOUT_CB(_devcb) \
	devcb = &rp5c15_device::set_out_clkout_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rp5c15_device

class rp5c15_device :   public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	rp5c15_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_alarm_callback(device_t &device, _Object object) { return downcast<rp5c15_device &>(device).m_out_alarm_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_clkout_callback(device_t &device, _Object object) { return downcast<rp5c15_device &>(device).m_out_clkout_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( adj_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_rtc_interface overrides
	virtual bool rtc_feature_leap_year() { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second);

private:
	inline void set_alarm_line();
	inline int read_counter(int counter);
	inline void write_counter(int counter, int value);
	inline void check_alarm();

	static const device_timer_id TIMER_CLOCK = 0;
	static const device_timer_id TIMER_16HZ = 1;
	static const device_timer_id TIMER_CLKOUT = 2;

	devcb_write_line        m_out_alarm_cb;
	devcb_write_line        m_out_clkout_cb;

	UINT8 m_reg[2][13];         // clock registers
	UINT8 m_ram[13];            // RAM

	UINT8 m_mode;               // mode register
	UINT8 m_reset;              // reset register
	int m_alarm;                // alarm output
	int m_alarm_on;             // alarm condition
	int m_1hz;                  // 1 Hz condition
	int m_16hz;                 // 16 Hz condition
	int m_clkout;               // clock output

	// timers
	emu_timer *m_clock_timer;
	emu_timer *m_16hz_timer;
	emu_timer *m_clkout_timer;
};


// device type definition
extern const device_type RP5C15;



#endif
