// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Dallas DS1386/DS1386P RAMified Watchdog Timekeeper

***********************************************************************
               _____________
    /INTA   1 |             | 28  Vcc
       NC   2 |             | 27  /WE
       NC   3 |             | 26  /INTB
       NC   4 |             | 25  NC
       A5   5 |             | 24  NC
       A4   6 |             | 23  SQW
       A3   7 |             | 22  /OE
       A2   8 |             | 21  NC
       A1   9 |             | 20  /CE
       A0  10 |             | 19  DQ7
      DQ0  11 |             | 18  DQ6
      DQ1  12 |             | 17  DQ5
      DQ2  13 |             | 16  DQ4
      GND  14 |_____________| 15  DQ3

               DS1286 (64 x 8)

               _____________
    /INTA   1 |             | 32  Vcc
    /INTB   2 |             | 31  SQW
   NC/A14   3 |             | 30  Vcc
      A12   4 |             | 29  /WE
       A7   5 |             | 28  NC/A13
       A6   6 |             | 27  A8
       A5   7 |             | 26  A9
       A4   8 |             | 25  A11
       A3   9 |             | 24  /OE
       A2  10 |             | 23  A10
       A1  11 |             | 22  /CE
       A0  12 |             | 21  DQ7
      DQ0  13 |             | 20  DQ6
      DQ1  14 |             | 19  DQ5
      DQ2  15 |             | 18  DQ4
      GND  16 |_____________| 17  DQ3

               DS1386 8k/32k x 8

             __________________________________
            /                                  |
           /                                   |
    /INTB |  1                              34 | /INTA
       NC |  2                              33 | SQW
       NC |  3                              32 | NC/A13
     /PFO |  4                              31 | NC/A14
      Vcc |  5                              30 | A12
      /WE |  6                              29 | A11
      /OE |  7                              28 | A10
      /CE |  8                              27 | A9
      DQ7 |  9                              26 | A8
      DQ6 | 10                              25 | A7
      DQ5 | 11                              24 | A6
      DQ4 | 12                              23 | A5
      DQ3 | 13                              22 | A4
      DQ2 | 14   X1     GND   Vbat    X2    21 | A3
      DQ1 | 15  ____   ____   ____   ____   20 | A2
      DQ0 | 16 |    | |    | |    | |    |  19 | A1
      GND | 17 |____| |____| |____| |____|  18 | A0
          |____________________________________|

     DS1386 8k/32k x 8, 34-Pin PowerCap Module Board

**********************************************************************/

#ifndef MAME_MACHINE_DS1386_H
#define MAME_MACHINE_DS1386_H

#pragma once

#include "dirtc.h"

class ds1386_device : public device_t,
					  public device_nvram_interface,
					  public device_rtc_interface
{
public:
	auto inta() { return m_inta_cb.bind(); }
	auto intb() { return m_intb_cb.bind(); }
	auto sqw() { return m_sqw_cb.bind(); }

	void data_w(offs_t offset, uint8_t data);
	uint8_t data_r(offs_t offset);

	void ce_w(int state);
	void oe_w(int state);
	void we_w(int state);

protected:
	enum
	{
		REGISTER_HUNDREDTHS = 0,
		REGISTER_SECONDS,
		REGISTER_MINUTES,
		REGISTER_MINUTE_ALARM,
		REGISTER_HOURS,
		REGISTER_HOUR_ALARM,
		REGISTER_DAYS,
		REGISTER_DAY_ALARM,
		REGISTER_DATE,
		REGISTER_MONTHS,
		REGISTER_EN_OUTS = REGISTER_MONTHS,
		REGISTER_YEARS,
		REGISTER_COMMAND,
		REGISTER_WATCHDOG_HUNDREDTHS,
		REGISTER_WATCHDOG_SECONDS,
		REGISTER_USER = 0xE,
	};

	enum
	{
		ALARM_DAYS_MATCH    = 0x0,
		ALARM_HOURS_MATCH   = 0x1,
		ALARM_MINUTES_MATCH = 0x3,
		ALARM_PER_MINUTE    = 0x7
	};

	ds1386_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, size_t size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

protected:
	enum
	{
		DISABLE_OSC     = 0x80,
		DISABLE_SQW     = 0x40,

		COMMAND_TE      = 0x80,
		COMMAND_IPSW    = 0x40,
		COMMAND_IBH_LO  = 0x20,
		COMMAND_PU_LVL  = 0x10,
		COMMAND_WAM     = 0x08,
		COMMAND_TDM     = 0x04,
		COMMAND_WAF     = 0x02,
		COMMAND_TDF     = 0x01,

		HOURS_12_24     = 0x40,
		HOURS_AM_PM     = 0x20
	};


	TIMER_CALLBACK_MEMBER(advance_hundredths);
	TIMER_CALLBACK_MEMBER(square_tick);
	TIMER_CALLBACK_MEMBER(watchdog_tick);
	TIMER_CALLBACK_MEMBER(inta_timer_elapsed);
	TIMER_CALLBACK_MEMBER(intb_timer_elapsed);

	void safe_inta_cb(int state);
	void safe_intb_cb(int state);
	void safe_sqw_cb(int state);

	void check_tod_alarm();
	void time_of_day_alarm();
	void watchdog_alarm();

	void copy_ram_to_registers();
	void copy_registers_to_ram();

	int m_tod_alarm;
	int m_watchdog_alarm;
	int m_square;

	// interfacing with other devices
	devcb_write_line    m_inta_cb;
	devcb_write_line    m_intb_cb;
	devcb_write_line    m_sqw_cb;

	// timers
	emu_timer *m_clock_timer;
	emu_timer *m_square_timer;
	emu_timer *m_watchdog_timer;
	emu_timer *m_inta_timer;
	emu_timer *m_intb_timer;

	std::unique_ptr<uint8_t[]> m_ram;
	optional_region_ptr<uint8_t> m_default_data;

	uint8_t m_hundredths;
	uint8_t m_seconds;
	uint8_t m_minutes;
	uint8_t m_minutes_alarm;
	uint8_t m_hours;
	uint8_t m_hours_alarm;
	uint8_t m_days;
	uint8_t m_days_alarm;
	uint8_t m_date;
	uint8_t m_months_enables;
	uint8_t m_years;

	const size_t m_ram_size;
};

class ds1286_device : public ds1386_device
{
public:
	ds1286_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ds1386_8k_device : public ds1386_device
{
public:
	ds1386_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ds1386_32k_device : public ds1386_device
{
public:
	ds1386_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(DS1286,     ds1286_device)
DECLARE_DEVICE_TYPE(DS1386_8K,  ds1386_8k_device)
DECLARE_DEVICE_TYPE(DS1386_32K, ds1386_32k_device)

#endif // MAME_MACHINE_DS1386_H
