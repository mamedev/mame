// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Philips PCF8573 Clock and Calendar with Power Fail Detector

**********************************************************************

    The PCF8573 comes in two package configurations:

        PCF8573P  - 16-pin dual-inline package (DIP16)
        PCF8573T  - 16-pin small-outline package (SO16)

**********************************************************************

    Pinning:                ____  ____
                           |    \/    |
                    A0   1 |          | 16  Vdd
                    A1   2 |          | 15  Vss1
                  COMP   3 |          | 14  OSC0
                   SDA   4 | PCF8573P | 13  OSC1
                   SDL   5 | PCF8573T | 12  TEST
                 EXTPF   6 |          | 11  FSET
                  PFIN   7 |          | 10  SEC
                  Vss2   8 |          | 9   MIN
                           |__________|

*********************************************************************/

#ifndef MAME_MACHINE_PCF8573_H
#define MAME_MACHINE_PCF8573_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pcf8573_device

class pcf8573_device :
	public device_t,
	public device_rtc_interface
{
public:
	pcf8573_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto comp_cb() { return m_comp_cb.bind(); }
	auto min_cb() { return m_min_cb.bind(); }
	auto sec_cb() { return m_sec_cb.bind(); }

	void set_a0(int a0) { m_slave_address = (m_slave_address & 0xfd) | (a0 << 1); }
	void set_a1(int a1) { m_slave_address = (m_slave_address & 0xfb) | (a1 << 2); }

	void a0_w(int state);
	void a1_w(int state);
	void scl_w(int state);
	void sda_w(int state);
	int sda_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

	TIMER_CALLBACK_MEMBER(clock_tick);

private:
	static constexpr uint8_t PCF8573_SLAVE_ADDRESS = 0xd0;

	enum
	{
		REG_HOURS               = 0x00,
		REG_MINUTES             = 0x01,
		REG_DAYS                = 0x02,
		REG_MONTHS              = 0x03,
		REG_ALARM_HOURS         = 0x04,
		REG_ALARM_MINUTES       = 0x05,
		REG_ALARM_DAYS          = 0x06,
		REG_ALARM_MONTHS        = 0x07
	};

	// get/set time
	uint8_t get_time_minute()           { return bcd_to_integer(m_data[REG_MINUTES]); }
	void set_time_minute(uint8_t minute){ m_data[REG_MINUTES] = convert_to_bcd(minute); }
	uint8_t get_time_hour()             { return bcd_to_integer(m_data[REG_HOURS]); }
	void set_time_hour(uint8_t hour)    { m_data[REG_HOURS] = convert_to_bcd(hour); }

	// get/set date
	uint8_t get_date_day()              { return bcd_to_integer(m_data[REG_DAYS]); }
	void set_date_day(uint8_t day)      { m_data[REG_DAYS] = convert_to_bcd(day); }
	uint8_t get_date_month()            { return bcd_to_integer(m_data[REG_MONTHS]); }
	void set_date_month(uint8_t month)  { m_data[REG_MONTHS] = convert_to_bcd(month); }

	devcb_write_line m_comp_cb;
	devcb_write_line m_min_cb;
	devcb_write_line m_sec_cb;

	// timers
	emu_timer *m_clock_timer;

	// internal state
	uint8_t m_data[8];
	int m_slave_address;
	int m_scl;
	int m_sdaw;
	int m_sdar;
	int m_state;
	int m_bits;
	int m_shift;
	int m_devsel;
	int m_mode_pointer;
	int m_address;
	int m_status;

	enum { STATE_IDLE, STATE_ADDRESS, STATE_MODE, STATE_DATAIN, STATE_DATAOUT, STATE_READSELACK };
};

// device type definition
DECLARE_DEVICE_TYPE(PCF8573, pcf8573_device)

#endif // MAME_MACHINE_PCF8573_H
