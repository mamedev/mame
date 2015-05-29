// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    MSM6242 Real Time Clock

***************************************************************************/

#pragma once

#ifndef __MSM6242DEV_H__
#define __MSM6242DEV_H__

#include "emu.h"


#define MCFG_MSM6242_OUT_INT_HANDLER(_devcb) \
	devcb = &msm6242_device::set_out_int_handler(*device, DEVCB_##_devcb);


// ======================> msm6242_device

class msm6242_device :  public device_t,
								public device_rtc_interface
{
public:
	// construction/destruction
	msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


	template<class _Object> static devcb_base &set_out_int_handler(device_t &device, _Object object) { return downcast<msm6242_device &>(device).m_out_int_handler.set_callback(object); }

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_pre_save();
	virtual void device_post_load();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// rtc overrides
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second);

private:
	static const int RTC_TICKS = ~0;

	static const UINT8 IRQ_64THSECOND = 0;
	static const UINT8 IRQ_SECOND = 1;
	static const UINT8 IRQ_MINUTE = 2;
	static const UINT8 IRQ_HOUR = 3;

	// state
	UINT8                       m_reg[3];
	UINT8                       m_irq_flag;
	UINT8                       m_irq_type;
	UINT16                      m_tick;

	// incidentals
	devcb_write_line m_out_int_handler;
	emu_timer *                 m_timer;
	UINT64                      m_last_update_time; // last update time, in clock cycles

	// methods
	void rtc_timer_callback();
	UINT64 current_time();
	void irq(UINT8 irq_type);
	UINT64 bump(int rtc_register, UINT64 delta, UINT64 register_min, UINT64 register_range);
	void update_rtc_registers();
	void update_timer();
	UINT8 get_clock_nibble(int rtc_register, bool high);
	static const char *irq_type_string(UINT8 irq_type);
};


// device type definition
extern const device_type MSM6242;


#endif /* __MSM6242DEV_H__ */
