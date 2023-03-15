// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    EM Microelectronic-Marin (µem) M 3002 Real Time Clock

***********************************************************************
                             ____    ____
                     Vbb  1 |*   \__/    | 16  Vdd
                      XI  2 |            | 15  _PULSE
                      XO  3 |            | 14  _BUSY
                   _SYNC  4 |            | 13  _IRQ
                    R/_W  5 |   M 3002   | 12  I/O 3
                     _OE  6 |            | 11  I/O 2
                     _CS  7 |            | 10  I/O 1
                     Vss  8 |____________|  9  I/O 0

**********************************************************************/

#ifndef MAME_MACHINE_M3002_H
#define MAME_MACHINE_M3002_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m3002_device

class m3002_device : public device_t, public device_nvram_interface, public device_rtc_interface
{
public:
	// device type constructor
	m3002_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_out() { return m_irq_callback.bind(); }

	// 4-bit read/write handlers
	u8 read();
	void write(u8 data);

	// status output polling
	DECLARE_READ_LINE_MEMBER(busy_r) { return internal_busy() ? 0 : 1; }
	DECLARE_READ_LINE_MEMBER(irq_r) { return m_irq_active ? 0 : 1; }

protected:
	m3002_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_clock_changed() override;

	// device_nvram_interface overrides
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual void nvram_default() override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return false; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	static const char *const s_register_names[0x10];

	enum class mux_state : u8
	{
		INIT,
		TENS,
		UNITS
	};

	// internal helpers
	bool internal_busy() const;
	void bcd_increment(u8 location);
	u8 max_date() const;
	void watch_update();
	void alarm_update();
	void timer_update();
	void irq_update();
	void update();
	TIMER_CALLBACK_MEMBER(second_timer);
	void set_init_state();

	// callback objects
	devcb_write_line m_irq_callback;

	// internal state
	u8 m_ram[16];
	u8 m_address;
	mux_state m_mux_state;
	bool m_irq_active;
	bool m_update_deferred;
	emu_timer *m_second_timer;
};

// ======================> m3000_device

class m3000_device : public m3002_device
{
public:
	// device type constructor
	m3000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(M3002, m3002_device)
DECLARE_DEVICE_TYPE(M3000, m3000_device)

#endif // MAME_MACHINE_M3002_H
