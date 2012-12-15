/***************************************************************************

    MSM6242 Real Time Clock

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __MSM6242DEV_H__
#define __MSM6242DEV_H__

#include "emu.h"
#include "dirtc.h"


#define MCFG_MSM6242_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, msm6242, XTAL_32_768kHz) \
	MCFG_DEVICE_CONFIG(_config)

#define MSM6242_INTERFACE(name) \
	const msm6242_interface (name) =

// ======================> msm6242_interface

struct msm6242_interface
{
	devcb_write_line	m_out_int_func;
};


// ======================> msm6242_device

class msm6242_device :	public device_t, public device_rtc_interface
{
public:
	// construction/destruction
	msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	UINT8						m_reg[3];
	UINT8						m_irq_flag;
	UINT8						m_irq_type;
	UINT16						m_tick;

	// incidentals
	devcb_resolved_write_line	m_res_out_int_func;
	emu_timer *					m_timer;
	UINT64						m_last_update_time;	// last update time, in clock cycles

	// methods
	void rtc_timer_callback();
	UINT64 current_time();
	void irq(UINT8 irq_type);
	UINT64 bump(int rtc_register, UINT64 delta, UINT64 register_min, UINT64 register_range);
	void update_rtc_registers();
	void update_timer();
	UINT8 get_clock_nibble(int rtc_register, bool high);
};


// device type definition
extern const device_type msm6242;


#endif /* __MSM6242DEV_H__ */
