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

**********************************************************************/

#pragma once

#ifndef __MSM58321__
#define __MSM58321__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MSM58321_D0_HANDLER(_devcb) \
	devcb = &msm58321_device::set_d0_handler(*device, DEVCB_##_devcb);

#define MCFG_MSM58321_D1_HANDLER(_devcb) \
	devcb = &msm58321_device::set_d1_handler(*device, DEVCB_##_devcb);

#define MCFG_MSM58321_D2_HANDLER(_devcb) \
	devcb = &msm58321_device::set_d2_handler(*device, DEVCB_##_devcb);

#define MCFG_MSM58321_D3_HANDLER(_devcb) \
	devcb = &msm58321_device::set_d3_handler(*device, DEVCB_##_devcb);

#define MCFG_MSM58321_BUSY_HANDLER(_devcb) \
	devcb = &msm58321_device::set_busy_handler(*device, DEVCB_##_devcb);

#define MCFG_MSM58321_YEAR0(_year0) \
	msm58321_device::set_year0(*device, _year0);

#define MCFG_MSM58321_DEFAULT_24H(_default_24h) \
	msm58321_device::set_default_24h(*device, _default_24h);

// ======================> msm58321_device

class msm58321_device : public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	msm58321_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_d0_handler(device_t &device, _Object object) { return downcast<msm58321_device &>(device).m_d0_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d1_handler(device_t &device, _Object object) { return downcast<msm58321_device &>(device).m_d1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d2_handler(device_t &device, _Object object) { return downcast<msm58321_device &>(device).m_d2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d3_handler(device_t &device, _Object object) { return downcast<msm58321_device &>(device).m_d3_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_busy_handler(device_t &device, _Object object) { return downcast<msm58321_device &>(device).m_busy_handler.set_callback(object); }
	static void set_year0(device_t &device, int year0) { downcast<msm58321_device &>(device).m_year0 = year0; }
	static void set_default_24h(device_t &device, bool default_24h) { downcast<msm58321_device &>(device).m_default_24h = default_24h; }

	DECLARE_WRITE_LINE_MEMBER( cs2_w );
	DECLARE_WRITE_LINE_MEMBER( write_w );
	DECLARE_WRITE_LINE_MEMBER( read_w );
	DECLARE_WRITE_LINE_MEMBER( d0_w );
	DECLARE_WRITE_LINE_MEMBER( d1_w );
	DECLARE_WRITE_LINE_MEMBER( d2_w );
	DECLARE_WRITE_LINE_MEMBER( d3_w );
	DECLARE_WRITE_LINE_MEMBER( address_write_w );
	DECLARE_WRITE_LINE_MEMBER( stop_w );
	DECLARE_WRITE_LINE_MEMBER( test_w );
	DECLARE_WRITE_LINE_MEMBER( cs1_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	virtual bool rtc_feature_y2k() override { return m_year0 != 0; }

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	static const device_timer_id TIMER_CLOCK = 0;
	static const device_timer_id TIMER_BUSY = 1;

	void update_input();
	void update_output();

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

	UINT8 m_address;            // address latch
	UINT8 m_reg[13];            // registers

	// timers
	emu_timer *m_clock_timer;
	emu_timer *m_busy_timer;
};


// device type definition
extern const device_type MSM58321;

#endif
