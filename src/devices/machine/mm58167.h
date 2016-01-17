// license:BSD-3-Clause
// copyright-holders:R. Belmont
/**********************************************************************

    mm58167.h - National Semiconductor MM58167 real-time clock emulation

**********************************************************************/

#pragma once

#ifndef __MM58167_H__
#define __MM58167_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MM58167_IRQ_CALLBACK(_cb) \
	devcb = &mm58167_device::set_irq_cb(*device, DEVCB_##_cb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mm58167_device

class mm58167_device :  public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	mm58167_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	template<class _Object> static devcb_base &set_irq_cb(device_t &device, _Object wr) { return downcast<mm58167_device &>(device).m_irq_w.set_callback(wr); }

	devcb_write_line m_irq_w;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_rtc_interface overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
	virtual bool rtc_feature_leap_year() override { return true; }

	void set_irq(int bit);
	void update_rtc();

private:
	int m_regs[32];
	int m_milliseconds;
	bool m_comparator_state;

	// timers
	emu_timer *m_clock_timer;
};

// device type definition
extern const device_type MM58167;

#endif
