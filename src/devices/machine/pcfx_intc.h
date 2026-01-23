// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese

#ifndef MAME_MACHINE_PCFX_INTC_H
#define MAME_MACHINE_PCFX_INTC_H

#pragma once

class pcfx_intc_device : public device_t
{
public:
	pcfx_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto int_cb() { return m_int_w.bind(); }

	u16 read(offs_t offset);
	void write(offs_t offset, u16 data);

	void irq8_w(int state)  { set_irq_line( 8, state); }
	void irq9_w(int state)  { set_irq_line( 9, state); }
	void irq10_w(int state) { set_irq_line(10, state); }
	void irq11_w(int state) { set_irq_line(11, state); }
	void irq12_w(int state) { set_irq_line(12, state); }
	void irq13_w(int state) { set_irq_line(13, state); }
	void irq14_w(int state) { set_irq_line(14, state); }
	void irq15_w(int state) { set_irq_line(15, state); }

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
