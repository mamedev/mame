// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation IRQ emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#pragma once

#ifndef __PSXIRQ_H__
#define __PSXIRQ_H__

#include "emu.h"

extern const device_type PSX_IRQ;

#define MCFG_PSX_IRQ_HANDLER(_devcb) \
	devcb = &psxirq_device::set_irq_handler(*device, DEVCB_##_devcb);

class psxirq_device : public device_t
{
public:
	psxirq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<psxirq_device &>(device).m_irq_handler.set_callback(object); }

	uint32_t read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void intin0(int state);
	void intin1(int state);
	void intin2(int state);
	void intin3(int state);
	void intin4(int state);
	void intin5(int state);
	void intin6(int state);
	void intin7(int state);
	void intin8(int state);
	void intin9(int state);
	void intin10(int state);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	void psx_irq_update( void );
	void set( uint32_t bitmask );

	uint32_t n_irqdata;
	uint32_t n_irqmask;

	devcb_write_line m_irq_handler;
};

#endif
