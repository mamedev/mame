// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    DP8573 Real Time Clock (RTC)

***************************************************************************/

#ifndef DEVICES_MACHINE_DP8573_H
#define DEVICES_MACHINE_DP8573_H

#pragma once

class dp8573_device : public device_t, public device_nvram_interface
{
public:
	dp8573_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: dp8573_device(mconfig, tag, owner, 32768)
	{
	}

	dp8573_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);
	void pfail_w(int state) {}

	auto intr() { return m_intr_cb.bind(); }
	auto mfo() { return m_mfo_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	void sync_time();
	void save_registers();
	void set_interrupt(uint8_t mask);
	void clear_interrupt(uint8_t mask);

	static const device_timer_id TIMER_ID = 0;

	enum
	{
		REG_MSR             = 0x00, // Main Status Register
		REG_RTMR            = 0x01, // Not Applicable / Real-Time Mode Register
		REG_OMR             = 0x02, // Not Applicable / Output Mode Register
		REG_PFR_ICR0        = 0x03, // Periodic Flag Register / Interrupt Control Register 0
		REG_TSCR_ICR1       = 0x04, // Time Save Control Register / Interrupt Control Register 1
		REG_HUNDREDTH       = 0x05, // Hundredths and Tenths of a Second (0-99)
		REG_SECOND          = 0x06, // Seconds (0-59)
		REG_MINUTE          = 0x07, // Minutes (0-59)
		REG_HOUR            = 0x08, // Hours (1-12, 0-23)
		REG_DAY             = 0x09, // Day of Month (1-28/29/30/31)
		REG_MONTH           = 0x0a, // Month (1-12)
		REG_YEAR            = 0x0b, // Year (0-99)
		// 0x0c - RAM
		REG_RAM_D1D0        = 0x0d, // RAM, D1/D0 bits only
		REG_DAYOFWEEK       = 0x0e, // Day of Week (1-7)
		REG_NA_0FH          = 0x0f,
		REG_NA_10H          = 0x10,
		REG_NA_11H          = 0x11,
		REG_NA_12H          = 0x12,
		REG_COMP_SECOND     = 0x13, // Seconds Compare RAM (0-59)
		REG_COMP_MINUTE     = 0x14, // Minutes Compare RAM (0-59)
		REG_COMP_HOUR       = 0x15, // Hours Compare RAM (1-12, 0-23)
		REG_COMP_DAY        = 0x16, // Day of Month Compare RAM (1-28/29/30/31)
		REG_COMP_MONTH      = 0x17, // Month Compare RAM (1-12)
		REG_COMP_DAYOFWEEK  = 0x18, // Day of Week Compare RAM (1-7)
		REG_SAVE_SECOND     = 0x19, // Seconds Time Save RAM
		REG_SAVE_MINUTE     = 0x1a, // Minutes Time Save RAM
		REG_SAVE_HOUR       = 0x1b, // Hours Time Save RAM
		REG_SAVE_DAY        = 0x1c, // Day of Month Time Save RAM
		REG_SAVE_MONTH      = 0x1d, // Month Time Save RAM
		// 0x1e - RAM
		REG_TEST            = 0x1f, // RAM / Test Mode Register

		MSR_INT             = 0x01, // Interrupt Status
		MSR_PF              = 0x02, // Power Fail Interrupt
		MSR_PER             = 0x04, // Period Interrupt
		MSR_AL              = 0x08, // Alarm Interrupt
		MSR_RS              = 0x40, // Register Select Bit
		MSR_RAM_MASK        = 0xf0,
		MSR_INT_MASK        = 0x0e,
		MSR_CLEARABLE_MASK  = 0x0c,

		PFR_1MIN            = 0x01, // Minutes flag
		PFR_10S             = 0x02, // 10-second flag
		PFR_1S              = 0x04, // Seconds flag
		PFR_100MS           = 0x08, // 100-millisecond flag
		PFR_10MS            = 0x10, // 10-millisecond flag
		PFR_1MS             = 0x20, // Millisecond flag
		PFR_OSF             = 0x40, // Oscillator Failed / Single Supply Bit
		PFR_TM              = 0x80, // Test Mode Enable
		PFR_READ_CLEAR_MASK = 0x3f,

		TSCR_RAM_MASK       = 0x3f,
		TSCR_NA             = 0x40, // N/A
		TSCR_TS             = 0x80, // Time Save Enable

		RTMR_LY0            = 0x01, // Leap Year LSB
		RTMR_LY1            = 0x02, // Leap Year MSB
		RTMR_LY             = 0x03,
		RTMR_12H            = 0x04, // 12/!24 hour mode
		RTMR_CSS            = 0x08, // Clock Start/!Stop
		RTMR_IPF            = 0x10, // Interrupt PF Operation
		RTMR_RAM_MASK       = 0xe0,

		OMR_RAM_MASK        = 0x7f,
		OMR_MO              = 0x80, // MFO Pin as Oscillator

		ICR0_MN             = 0x01, // Minutes enable
		ICR0_TS             = 0x02, // 10-second enable
		ICR0_S              = 0x04, // Seconds enable
		ICR0_HM             = 0x08, // 100 millisecond enable
		ICR0_TM             = 0x10, // 10 millisecond enable
		ICR0_1M             = 0x20, // Milliseconds enable
		ICR0_RAM_MASK       = 0xc0,

		ICR1_SC             = 0x01, // Second compare enable
		ICR1_MN             = 0x02, // Minute compare enable
		ICR1_HR             = 0x04, // Hour compare enable
		ICR1_DOM            = 0x08, // Day of month compare enable
		ICR1_MO             = 0x10, // Month compare enable
		ICR1_DOW            = 0x20, // Day of week compare enable
		ICR1_ALE            = 0x40, // Alarm interrupt enable
		ICR1_PFE            = 0x80, // Power fail interrupt enable
		ICR1_COMPARE_MASK   = 0x3f
	};

	uint8_t m_ram[32];
	uint8_t m_tscr;
	uint8_t m_pfr;
	uint8_t m_millis;

	emu_timer *m_timer;

	devcb_write_line m_intr_cb;
	devcb_write_line m_mfo_cb;
};

DECLARE_DEVICE_TYPE(DP8573, dp8573_device)

#endif // DEVICES_MACHINE_DP8573_H
