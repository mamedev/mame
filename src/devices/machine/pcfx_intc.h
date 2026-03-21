// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese

#ifndef MAME_MACHINE_PCFX_INTC_H
#define MAME_MACHINE_PCFX_INTC_H

#pragma once

#include <cassert>


class pcfx_intc_device : public device_t
{
public:
	pcfx_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto int_cb() { return m_int_w.bind(); }

	u16 read(offs_t offset);
	void write(offs_t offset, u16 data);

	template <unsigned Which> void irq_w(int state)  { static_assert((Which >= 8) && (Which <= 15)); set_irq_line(Which, state); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_write8 m_int_w;

	u16 m_irq_mask;
	u16 m_irq_pending;
	u8 m_irq_priority[8];

	void check_irqs();
	void set_irq_line(int line, int state);
};

DECLARE_DEVICE_TYPE(PCFX_INTC, pcfx_intc_device)

#endif // MAME_MACHINE_PCFX_INTC_H
