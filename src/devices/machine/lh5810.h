// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    LH5810/LH5811 Input/Output Port Controller

**********************************************************************/

#ifndef MAME_MACHINE_LH5810_H
#define MAME_MACHINE_LH5810_H

#pragma once

class lh5810_device : public device_t
{
public:
	// construction/destruction
	lh5810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto porta_r() { return m_porta_r_cb.bind(); }
	auto porta_w() { return m_porta_w_cb.bind(); }
	auto portb_r() { return m_portb_r_cb.bind(); }
	auto portb_w() { return m_portb_w_cb.bind(); }
	auto portc_w() { return m_portc_w_cb.bind(); }
	auto out_int() { return m_out_int_cb.bind(); }

	uint8_t data_r(offs_t offset);
	void data_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:

	devcb_read8         m_porta_r_cb;       //port A read
	devcb_write8        m_porta_w_cb;       //port A write
	devcb_read8         m_portb_r_cb;       //port B read
	devcb_write8        m_portb_w_cb;       //port B write
	devcb_write8        m_portc_w_cb;       //port C write

	devcb_write_line    m_out_int_cb;       //IRQ callback

	uint8_t m_reg[0x10];
	uint8_t m_irq;
};

DECLARE_DEVICE_TYPE(LH5810, lh5810_device)

#endif // MAME_MACHINE_LH5810_H
