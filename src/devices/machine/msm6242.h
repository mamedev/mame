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

#pragma once

#ifndef __MSM6242DEV_H__
#define __MSM6242DEV_H__

#include "dirtc.h"


#define MCFG_MSM6242_OUT_INT_HANDLER(_devcb) \
	devcb = &msm6242_device::set_out_int_handler(*device, DEVCB_##_devcb);


// ======================> msm6242_device

class msm6242_device :  public device_t,
								public device_rtc_interface
{
public:
	// construction/destruction
	msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	msm6242_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *filename);

	template<class _Object> static devcb_base &set_out_int_handler(device_t &device, _Object object) { return downcast<msm6242_device &>(device).m_out_int_handler.set_callback(object); }

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// rtc overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	static const int RTC_TICKS = ~0;

	static const uint8_t IRQ_64THSECOND = 0;
	static const uint8_t IRQ_SECOND = 1;
	static const uint8_t IRQ_MINUTE = 2;
	static const uint8_t IRQ_HOUR = 3;

	// state
	uint8_t                       m_reg[3];
	uint8_t                       m_irq_flag;
	uint8_t                       m_irq_type;
	uint16_t                      m_tick;

	// incidentals
	devcb_write_line m_out_int_handler;
	emu_timer *                 m_timer;
	uint64_t                      m_last_update_time; // last update time, in clock cycles

	// methods
	void rtc_timer_callback();
	uint64_t current_time();
	void irq(uint8_t irq_type);
	uint64_t bump(int rtc_register, uint64_t delta, uint64_t register_min, uint64_t register_range);
	void update_rtc_registers();
	void update_timer();
	uint8_t get_clock_nibble(int rtc_register, bool high);
	static const char *irq_type_string(uint8_t irq_type);
};

// ======================> rtc62421_device

class rtc62421_device : public msm6242_device
{
public:
	// construction/destruction
	rtc62421_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> rtc62423_device

class rtc62423_device : public msm6242_device
{
public:
	// construction/destruction
	rtc62423_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> rtc72421_device

class rtc72421_device : public msm6242_device
{
public:
	// construction/destruction
	rtc72421_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> rtc72423_device

class rtc72423_device : public msm6242_device
{
public:
	// construction/destruction
	rtc72423_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
extern const device_type MSM6242;
extern const device_type RTC62421;
extern const device_type RTC62423;
extern const device_type RTC72421;
extern const device_type RTC72423;


#endif /* __MSM6242DEV_H__ */
