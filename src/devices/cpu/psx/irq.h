// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation IRQ emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#ifndef MAME_CPU_PSX_IRQ_H
#define MAME_CPU_PSX_IRQ_H

#pragma once


DECLARE_DEVICE_TYPE(PSX_IRQ, psxirq_device)

class psxirq_device : public device_t
{
public:
	psxirq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq() { return m_irq_handler.bind(); }

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	void psx_irq_update( void );
	void set( uint32_t bitmask );

	uint32_t n_irqdata;
	uint32_t n_irqmask;

	devcb_write_line m_irq_handler;
};

#endif // MAME_CPU_PSX_IRQ_H
