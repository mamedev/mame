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

#ifndef MAME_MACHINE_RP5C15_H
#define MAME_MACHINE_RP5C15_H

#pragma once

#include "dirtc.h"

class rp5c15_device :   public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	rp5c15_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto alarm() { return m_out_alarm_cb.bind(); }
	auto clkout() { return m_out_clkout_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

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

	uint8_t m_reg[2][13];         // clock registers
	uint8_t m_ram[13];            // RAM

	uint8_t m_mode;               // mode register
	uint8_t m_reset;              // reset register
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
DECLARE_DEVICE_TYPE(RP5C15, rp5c15_device)

#endif // MAME_MACHINE_RP5C15_H
