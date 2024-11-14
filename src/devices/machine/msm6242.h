// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    MSM6242 / RTC-6242x / RTC-7242x Real Time Clock

****************************************************************************
                            _____   _____
                 STD.P   1 |*    \_/     | 18  Vdd
                  /CS0   2 |             | 17  /XT (MSM6242 only)
                   ALE   3 |             | 16  XT  (MSM6242 only)
                    A0   4 |  MSM6242RS  | 15  CS1
                    A1   5 |  RTC62421   | 14  D0
                    A2   6 |  RTC72421   | 13  D1
                    A3   7 |             | 12  D2
                   /RD   8 |             | 11  D3
                   GND   9 |_____________| 10  /WR

                            _____   _____
                 STD.P   1 |*    \_/     | 24  Vdd
                  /CS0   2 |             | 23  /XT (MSM6242 only)
                    NC   3 |             | 22  XT  (MSM6242 only)
                   ALE   4 |             | 21  NC
                    A0   5 |             | 20  CS1
                    NC   6 |  MSM6242GS  | 19  D0
                    A1   7 |  RTC62423   | 18  NC
                    NC   8 |  RTC72423   | 17  NC
                    A2   9 |             | 16  D1
                    A3  10 |             | 15  D2
                   /RD  11 |             | 14  D3
                   GND  12 |_____________| 13  /WR

***************************************************************************/

#ifndef MAME_MACHINE_MSM6242_H
#define MAME_MACHINE_MSM6242_H

#pragma once

#include "dirtc.h"


// ======================> msm6242_device

class msm6242_device : public device_t, public device_rtc_interface
{
public:
	// construction/destruction
	msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto out_int_handler() { return m_out_int_handler.bind(); }

	// I/O operations
	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

protected:
	msm6242_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;

	// rtc overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	static constexpr int RTC_TICKS = ~0;

	static constexpr u8 IRQ_64THSECOND = 0;
	static constexpr u8 IRQ_SECOND = 1;
	static constexpr u8 IRQ_MINUTE = 2;
	static constexpr u8 IRQ_HOUR = 3;

	// state
	u8                          m_reg[3];
	u8                          m_irq_flag;
	u8                          m_irq_type;
	u16                         m_tick;

	// incidentals
	devcb_write_line m_out_int_handler;
	emu_timer *                 m_timer;
	emu_timer *                 m_timer_irq_clear;
	u64                         m_last_update_time; // last update time, in clock cycles

	// methods
	TIMER_CALLBACK_MEMBER(rtc_timer_callback);
	TIMER_CALLBACK_MEMBER(rtc_irq_pulse_timer_callback);
	u64 current_time();
	void set_irq(bool active);
	void irq(u8 irq_type);
	u64 bump(int rtc_register, u64 delta, u64 register_min, u64 register_range);
	void update_rtc_registers();
	void update_timer();
	u8 get_clock_nibble(int rtc_register, bool high);
	static const char *irq_type_string(u8 irq_type);
};

// ======================> rtc62421_device

class rtc62421_device : public msm6242_device
{
public:
	// construction/destruction
	rtc62421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> rtc62423_device

class rtc62423_device : public msm6242_device
{
public:
	// construction/destruction
	rtc62423_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> rtc72421_device

class rtc72421_device : public msm6242_device
{
public:
	// construction/destruction
	rtc72421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> rtc72423_device

class rtc72423_device : public msm6242_device
{
public:
	// construction/destruction
	rtc72423_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


// device type definition
DECLARE_DEVICE_TYPE(MSM6242,  msm6242_device)
DECLARE_DEVICE_TYPE(RTC62421, rtc62421_device)
DECLARE_DEVICE_TYPE(RTC62423, rtc62423_device)
DECLARE_DEVICE_TYPE(RTC72421, rtc72421_device)
DECLARE_DEVICE_TYPE(RTC72423, rtc72423_device)


#endif // MAME_MACHINE_MSM6242_H
