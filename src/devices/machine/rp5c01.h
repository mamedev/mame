// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Ricoh RP5C01(A) Real Time Clock With Internal RAM emulation

**********************************************************************
                            _____   _____
                   _CS   1 |*    \_/     | 18  Vcc
                    CS   2 |             | 17  OSCOUT
                   ADJ   3 |             | 16  OSCIN
                    A0   4 |   RP5C01    | 15  _ALARM
                    A1   5 |   RP5C01A   | 14  D3
                    A2   6 |   RF5C01A   | 13  D2
                    A3   7 |   TC8521    | 12  D1
                   _RD   8 |             | 11  D0
                   GND   9 |_____________| 10  _WR

**********************************************************************/

#pragma once

#ifndef __RP5C01__
#define __RP5C01__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RP5C01_OUT_ALARM_CB(_devcb) \
	devcb = &rp5c01_device::set_out_alarm_callback(*device, DEVCB_##_devcb);

// include this macro if the chip is not battery backed
#define MCFG_RP5C01_REMOVE_BATTERY() \
	rp5c01_device::remove_battery(*device);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rp5c01_device

class rp5c01_device :   public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	rp5c01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_alarm_callback(device_t &device, _Object object) { return downcast<rp5c01_device &>(device).m_out_alarm_cb.set_callback(object); }
	static void remove_battery(device_t &device) { downcast<rp5c01_device &>(device).m_battery_backed = false; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ_LINE_MEMBER( alarm_r ) { return m_alarm; }
	DECLARE_WRITE_LINE_MEMBER( adj_w ) { if (state) adjust_seconds(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_leap_year() override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	inline void set_alarm_line();
	inline int read_counter(int counter);
	inline void write_counter(int counter, int value);
	inline void check_alarm();

	static const device_timer_id TIMER_CLOCK = 0;
	static const device_timer_id TIMER_16HZ = 1;

	devcb_write_line m_out_alarm_cb;
	bool m_battery_backed;

	UINT8 m_reg[2][13];         // clock registers
	UINT8 m_ram[13];            // RAM

	UINT8 m_mode;               // mode register
	UINT8 m_reset;              // reset register
	int m_alarm;                // alarm output
	int m_alarm_on;             // alarm condition
	int m_1hz;                  // 1 Hz condition
	int m_16hz;                 // 16 Hz condition

	// timers
	emu_timer *m_clock_timer;
	emu_timer *m_16hz_timer;
};


// device type definition
extern const device_type RP5C01;



#endif
