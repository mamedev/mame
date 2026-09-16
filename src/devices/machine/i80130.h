// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 80130 iRMX Operating System Processor emulation

**********************************************************************/

#ifndef MAME_MACHINE_I80130_H
#define MAME_MACHINE_I80130_H

#pragma once

#include "machine/pic8259.h"
#include "machine/pit8253.h"

class i80130_device :  public device_t
{
public:
	// construction/destruction
	i80130_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq() { return m_write_irq.bind(); }
	auto ack() { return m_write_ack.bind(); }
	auto lir() { return m_write_lir.bind(); }
	auto systick() { return m_write_systick.bind(); }
	auto delay() { return m_write_delay.bind(); }
	auto baud() { return m_write_baud.bind(); }

	virtual void rom_map(address_map &map) ATTR_COLD;
	virtual void io_map(address_map &map) ATTR_COLD;

	uint8_t inta_r() { return m_pic->acknowledge(); }

	void ir0_w(int state) { m_pic->ir0_w(state); }
	void ir1_w(int state) { m_pic->ir1_w(state); }
	void ir2_w(int state) { m_pic->ir2_w(state); }
	void ir3_w(int state) { m_pic->ir3_w(state); }
	void ir4_w(int state) { m_pic->ir4_w(state); }
	void ir5_w(int state) { m_pic->ir5_w(state); }
	void ir6_w(int state) { m_pic->ir6_w(state); }
	void ir7_w(int state) { m_pic->ir7_w(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
//  virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<pic8259_device> m_pic;
	required_device<pit8254_device> m_pit;

	devcb_write_line m_write_irq;
	devcb_write_line m_write_ack;
	devcb_write_line m_write_lir;
	devcb_write_line m_write_systick;
	devcb_write_line m_write_delay;
	devcb_write_line m_write_baud;

	void irq_w(int state) { m_write_irq(state); }
	void systick_w(int state) { m_write_systick(state); }
	void delay_w(int state) { m_write_delay(state); }
	void baud_w(int state) { m_write_baud(state); }

	uint16_t io_r(offs_t offset, uint16_t mem_mask = ~0);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
};


// device type definition
DECLARE_DEVICE_TYPE(I80130, i80130_device)

#endif // MAME_MACHINE_I80130_H
