// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_INTC_H
#define MAME_MACHINE_XAVIX_INTC_H

#pragma once

class xavix_intc_device : public device_t
{
public:
	enum irq_source : u8
	{
		IRQ_ADC = 0x04,
		IRQ_IO = 0x08,
		IRQ_TIMER = 0x10,
		IRQ_DMA = 0x20,
		IRQ_VIDEO = 0x40,
		IRQ_SOUND = 0x80
	};

	xavix_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto irq_callback() { return m_irq_cb.bind(); }
	void map(address_map &map) ATTR_COLD;
	s16 interrupt_vector_r(int source, int low_byte);

	template <u8 Mask> void irq_w(int state)
	{
		if (state)
			m_irq_source |= Mask;
		else
			m_irq_source &= ~Mask;
		update_irq_line();
	}

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update_irq_line();
	u8 irq_source_r();
	void irq_source_w(u8 data);
	u8 vector_enable_r();
	void vector_enable_w(u8 data);
	u8 nmi_vector_lo_r();
	void nmi_vector_lo_w(u8 data);
	u8 nmi_vector_hi_r();
	void nmi_vector_hi_w(u8 data);
	u8 irq_vector_lo_r();
	void irq_vector_lo_w(u8 data);
	u8 irq_vector_hi_r();
	void irq_vector_hi_w(u8 data);

	devcb_write_line m_irq_cb;
	u8 m_irq_source;
	u8 m_vector_enable;
	u8 m_nmi_vector_lo;
	u8 m_nmi_vector_hi;
	u8 m_irq_vector_lo;
	u8 m_irq_vector_hi;
};

DECLARE_DEVICE_TYPE(XAVIX_INTC, xavix_intc_device)

#endif // MAME_MACHINE_XAVIX_INTC_H
