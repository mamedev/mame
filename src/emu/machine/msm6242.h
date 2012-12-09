/***************************************************************************

    MSM6242 Real Time Clock

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __MSM6242DEV_H__
#define __MSM6242DEV_H__

#include "emu.h"


#define MCFG_MSM6242_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, msm6242, XTAL_32_768kHz) \
	MCFG_DEVICE_CONFIG(_config)

#define MSM6242_INTERFACE(name) \
	const msm6242_interface (name) =

// ======================> msm6242_interface

struct msm6242_interface
{
	devcb_write_line	m_out_int_cb;
};

struct rtc_regs_t
{
	UINT8 sec, min, hour, day, wday, month;
	UINT16 year;
};


// ======================> msm6242_device

class msm6242_device :	public device_t,
						public msm6242_interface
{
public:
	// construction/destruction
	msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	UINT8						m_reg[3];
	UINT8						m_irq_flag;
	UINT8						m_irq_type;
	UINT16						m_tick;

	rtc_regs_t					m_rtc;
	rtc_regs_t					m_hold;
	devcb_resolved_write_line	m_out_int_func;
	emu_timer *					m_timer;

	void rtc_timer_callback();
};


// device type definition
extern const device_type msm6242;


#endif /* __MSM6242DEV_H__ */
