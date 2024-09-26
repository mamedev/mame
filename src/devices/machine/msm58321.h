// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OKI MSM58321RS Real Time Clock/Calendar emulation

**********************************************************************
                            _____   _____
                   CS2   1 |*    \_/     | 16  Vdd
                 WRITE   2 |             | 15  XT
                  READ   3 |             | 14  _XT
                    D0   4 |   MSM58321  | 13  CS1
                    D1   5 |   RTC58321  | 12  TEST
                    D2   6 |             | 11  STOP
                    D3   7 |             | 10  _BUSY
                   GND   8 |_____________| 9   ADDRESS WRITE

                            _____   _____
                    NC   1 |*    \_/     | 24  Vdd
                    NC   2 |             | 23  Vdd
                    NC   3 |             | 22  Vdd
                    NC   4 |             | 21  Vdd
                   CS2   5 |             | 20  Vdd
                 WRITE   6 |             | 19  Vdd
                  READ   7 |   RTC58323  | 18  Vdd
                    D0   8 |             | 17  CS1
                    D1   9 |             | 16  TEST
                    D2  10 |             | 15  STOP
                    D3  11 |             | 14  _BUSY
                   GND  12 |_____________| 13  ADDRESS WRITE

**********************************************************************/

#ifndef MAME_MACHINE_MSM58321_H
#define MAME_MACHINE_MSM58321_H

#pragma once

#include "dirtc.h"


// ======================> msm58321_device

class msm58321_device : public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	msm58321_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto d0_handler() { return m_d0_handler.bind(); }
	auto d1_handler() { return m_d1_handler.bind(); }
	auto d2_handler() { return m_d2_handler.bind(); }
	auto d3_handler() { return m_d3_handler.bind(); }
	auto busy_handler() { return m_busy_handler.bind(); }
	void set_year0(int year0) { m_year0 = year0; }
	void set_default_24h(bool default_24h) { m_default_24h = default_24h; }

	void cs2_w(int state);
	void write_w(int state);
	void read_w(int state);
	void d0_w(int state);
	void d1_w(int state);
	void d2_w(int state);
	void d3_w(int state);
	void address_write_w(int state);
	void stop_w(int state);
	void test_w(int state);
	void cs1_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	virtual bool rtc_feature_y2k() const override { return m_year0 != 0; }

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual bool rtc_feature_leap_year() const override { return true; }

	TIMER_CALLBACK_MEMBER(clock_tick);
	TIMER_CALLBACK_MEMBER(release_busy);
	TIMER_CALLBACK_MEMBER(assert_standard);

private:
	void update_input();
	void update_output();
	void update_standard();
	inline int read_counter(int counter);
	inline void write_counter(int counter, int value);

	int m_year0;
	bool m_default_24h;
	devcb_write_line m_d0_handler;
	devcb_write_line m_d1_handler;
	devcb_write_line m_d2_handler;
	devcb_write_line m_d3_handler;
	devcb_write_line m_busy_handler;

	int m_cs2;                  // chip select 2
	int m_write;                // write data
	int m_read;                 // read data
	int m_d0_in;                // d0
	int m_d0_out;               // d0
	int m_d1_in;                // d1
	int m_d1_out;               // d1
	int m_d2_in;                // d2
	int m_d2_out;               // d2
	int m_d3_in;                // d3
	int m_d3_out;               // d3
	int m_address_write;        // write address
	int m_busy;                 // busy flag
	int m_stop;                 // stop flag
	int m_test;                 // test flag
	int m_cs1;                  // chip select 1

	uint8_t m_address;            // address latch
	std::array<uint8_t, 16> m_reg;            // registers

	// timers
	emu_timer *m_clock_timer;
	emu_timer *m_busy_timer;
	emu_timer *m_standard_timer;

	int m_khz_ctr;
};


// device type definition
DECLARE_DEVICE_TYPE(MSM58321, msm58321_device)

#endif
