// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    Dallas DS1386/DS1386P RAMified Watchdog Timekeeper

***********************************************************************
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


// handlers

#define MCFG_DS1386_INTA_HANDLER(_devcb) \
	devcb = &downcast<ds1386_device &>(*device).set_inta_cb(DEVCB_##_devcb);

#define MCFG_DS1386_INTB_HANDLER(_devcb) \
	devcb = &downcast<ds1386_device &>(*device).set_inta_cb(DEVCB_##_devcb);

#define MCFG_DS1386_SQW_HANDLER(_devcb) \
	devcb = &downcast<ds1386_device &>(*device).set_sqw_cb(DEVCB_##_devcb);

// devices

#define MCFG_DS1386_8K_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, DS1386_8K, _clock)

#define MCFG_DS1386_32K_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, DS1386_32K, _clock)

class ds1386_device : public device_t,
					  public device_nvram_interface
{
public:
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( data_r );

	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( oe_w );
	DECLARE_WRITE_LINE_MEMBER( we_w );

	template <class Object> devcb_base &set_inta_cb(Object &&cb) { return m_inta_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_intb_cb(Object &&cb) { return m_intb_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_sqw_cb(Object &&cb) { return m_sqw_cb.set_callback(std::forward<Object>(cb)); }

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
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

	static constexpr device_timer_id CLOCK_TIMER = 0;
	static constexpr device_timer_id SQUAREWAVE_TIMER = 1;
	static constexpr device_timer_id WATCHDOG_TIMER = 2;
	static constexpr device_timer_id INTA_TIMER = 3;
	static constexpr device_timer_id INTB_TIMER = 4;

protected:
	void safe_inta_cb(int state);
	void safe_intb_cb(int state);
	void safe_sqw_cb(int state);

	void set_current_time();

	void check_tod_alarm();
	void time_of_day_alarm();
	void watchdog_alarm();

	void advance_hundredths();

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

class ds1386_8k_device : public ds1386_device
{
public:
	// construction/destruction
	ds1386_8k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class ds1386_32k_device : public ds1386_device
{
public:
	// construction/destruction
	ds1386_32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(DS1386_8K,  ds1386_8k_device)
DECLARE_DEVICE_TYPE(DS1386_32K, ds1386_32k_device)

#endif // MAME_MACHINE_DS1386_H
