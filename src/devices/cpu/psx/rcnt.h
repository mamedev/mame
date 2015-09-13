// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation Root Counter emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#pragma once

#ifndef __PSXRCNT_H__
#define __PSXRCNT_H__

#include "emu.h"

extern const device_type PSX_RCNT;

#define MCFG_PSX_RCNT_IRQ0_HANDLER(_devcb) \
	devcb = &psxrcnt_device::set_irq0_handler(*device, DEVCB_##_devcb);
#define MCFG_PSX_RCNT_IRQ1_HANDLER(_devcb) \
	devcb = &psxrcnt_device::set_irq1_handler(*device, DEVCB_##_devcb);
#define MCFG_PSX_RCNT_IRQ2_HANDLER(_devcb) \
	devcb = &psxrcnt_device::set_irq2_handler(*device, DEVCB_##_devcb);
#define PSX_RC_STOP ( 0x01 )
#define PSX_RC_RESET ( 0x04 ) /* guess */
#define PSX_RC_COUNTTARGET ( 0x08 )
#define PSX_RC_IRQTARGET ( 0x10 )
#define PSX_RC_IRQOVERFLOW ( 0x20 )
#define PSX_RC_REPEAT ( 0x40 )
#define PSX_RC_CLC ( 0x100 )
#define PSX_RC_DIV ( 0x200 )

struct psx_root
{
	emu_timer *timer;
	UINT16 n_count;
	UINT16 n_mode;
	UINT16 n_target;
	UINT64 n_start;
};

class psxrcnt_device : public device_t
{
public:
	psxrcnt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq0_handler(device_t &device, _Object object) { return downcast<psxrcnt_device &>(device).m_irq0_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_irq1_handler(device_t &device, _Object object) { return downcast<psxrcnt_device &>(device).m_irq1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_irq2_handler(device_t &device, _Object object) { return downcast<psxrcnt_device &>(device).m_irq2_handler.set_callback(object); }

	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	psx_root root_counter[ 3 ];

	UINT64 gettotalcycles( void );
	int root_divider( int n_counter );
	UINT16 root_current( int n_counter );
	int root_target( int n_counter );
	void root_timer_adjust( int n_counter );

	devcb_write_line m_irq0_handler;
	devcb_write_line m_irq1_handler;
	devcb_write_line m_irq2_handler;
};

#endif
