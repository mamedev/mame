// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    macrtc.h - Apple 343-0042 real time clock and battery RAM
    by R. Belmont

**********************************************************************/

#ifndef MAME_APPLE_MACRTC_H
#define MAME_APPLE_MACRTC_H

#pragma once

#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rtc3430042_device

class rtc3430042_device :  public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
	friend class rtc3430040_device;

public:
	// construction/destruction
	rtc3430042_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool hasBigPRAM);
	rtc3430042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ce_w(int state);
	void clk_w(int state);
	int data_r();
	void data_w(int state);

	// 1 second square wave output
	auto cko_cb() { return m_cko_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	virtual bool rtc_feature_leap_year() const override { return true; }

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	TIMER_CALLBACK_MEMBER(half_seconds_tick);

private:
	bool m_is_big_PRAM;
	bool m_time_was_set;

	devcb_write_line m_cko_cb;

	/* state of rTCEnb and rTCClk lines */
	u8 m_rTCEnb = 0;
	u8 m_rTCClk = 0;

	/* serial transmit/receive register : bits are shifted in/out of this byte */
	u8 m_data_byte = 0;
	/* serial transmitted/received bit count */
	u8 m_bit_count = 0;
	/* direction of the current transfer (0 : VIA->RTC, 1 : RTC->VIA) */
	u8 m_data_dir = 0;
	/* when rtc_data_dir == 1, state of rTCData as set by RTC (-> data bit seen by VIA) */
	u8 m_data_out = 0;

	/* set to 1 when command in progress */
	u8 m_cmd = 0;

	/* write protect flag */
	u8 m_write_protect = 0;

	// test mode flag
	u8 m_test_mode = 0;

	/* internal seconds register */
	u8 m_seconds[/*8*/4]{};
	/* 20-byte long PRAM, or 256-byte long XPRAM */
	u8 m_pram[256]{};
	/* current extended address and RTC state */
	u8 m_xpaddr = 0;
	u8 m_state = 0;
	u8 m_data_latch = 0;
	bool m_cko = false;

	// timers
	emu_timer *m_clock_timer = nullptr;

	void rtc_shift_data(int data);
	void rtc_execute_cmd(int data);
};

class rtc3430040_device: public rtc3430042_device
{
public:
	rtc3430040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
};


// device type definition
DECLARE_DEVICE_TYPE(RTC3430040, rtc3430040_device)
DECLARE_DEVICE_TYPE(RTC3430042, rtc3430042_device)

#endif // MAME_APPLE_MACRTC_H
