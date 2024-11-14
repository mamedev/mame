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

#ifndef MAME_MACHINE_RP5C01_H
#define MAME_MACHINE_RP5C01_H

#pragma once

#include "dirtc.h"


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
	rp5c01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_alarm_callback() { return m_out_alarm_cb.bind(); }
	void remove_battery() { m_battery_backed = false; nvram_enable_backup(false); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	int alarm_r() { return m_alarm; }
	void adj_w(int state) { if (state) adjust_seconds(); }

protected:
	// construction/destruction
	rp5c01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual bool rtc_battery_backed() const override { return m_battery_backed; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	virtual TIMER_CALLBACK_MEMBER(advance_1hz_clock);
	TIMER_CALLBACK_MEMBER(advance_16hz_clock);

	inline void set_alarm_line();
private:
	inline int read_counter(int counter);
	inline void write_counter(int counter, int value);
	inline void check_alarm();

	devcb_write_line m_out_alarm_cb;
	bool m_battery_backed;

protected:
	uint8_t m_reg[2][13];         // clock registers
	uint8_t m_ram[13];            // RAM

	uint8_t m_mode;               // mode register
	uint8_t m_reset;              // reset register
	int m_alarm;                // alarm output
	int m_alarm_on;             // alarm condition
	int m_1hz;                  // 1 Hz condition
	int m_16hz;                 // 16 Hz condition

private:
	// timers
	emu_timer *m_clock_timer;
	emu_timer *m_16hz_timer;
};

// ======================> tc8521_device

class tc8521_device : public rp5c01_device
{
public:
	// construction/destruction
	tc8521_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class lh5045_device : public rp5c01_device
{
public:
	// construction/destruction
	lh5045_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	virtual TIMER_CALLBACK_MEMBER(advance_1hz_clock) override;
};

// device type definition
DECLARE_DEVICE_TYPE(RP5C01, rp5c01_device)
DECLARE_DEVICE_TYPE(TC8521, tc8521_device)
DECLARE_DEVICE_TYPE(LH5045, lh5045_device)

#endif // MAME_MACHINE_RP5C01_H
