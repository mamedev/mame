// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_MACHINE_M24_Z8000_H
#define MAME_MACHINE_M24_Z8000_H

#pragma once

#include "cpu/z8000/z8000.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"


class m24_z8000_device :  public device_t
{
public:
	m24_z8000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto halt_callback() { return m_halt_out.bind(); }

	DECLARE_READ16_MEMBER(pmem_r);
	DECLARE_WRITE16_MEMBER(pmem_w);
	DECLARE_READ16_MEMBER(dmem_r);
	DECLARE_WRITE16_MEMBER(dmem_w);
	DECLARE_READ16_MEMBER(i86_io_r);
	DECLARE_WRITE16_MEMBER(i86_io_w);
	DECLARE_WRITE8_MEMBER(irqctl_w);
	DECLARE_WRITE8_MEMBER(serctl_w);
	DECLARE_READ8_MEMBER(handshake_r);
	DECLARE_WRITE8_MEMBER(handshake_w);

	DECLARE_WRITE_LINE_MEMBER(halt_w) { m_z8000->set_input_line(INPUT_LINE_HALT, state); }
	DECLARE_WRITE_LINE_MEMBER(int_w) { m_z8000->set_input_line(INPUT_LINE_IRQ1, state); }

	bool halted() const { return m_z8000_halt; }

	void z8000_data(address_map &map);
	void z8000_io(address_map &map);
	void z8000_prog(address_map &map);
protected:
	void device_start() override;
	void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<z8001_device> m_z8000;
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	devcb_write_line m_halt_out;
	static const uint8_t pmem_table[16][4];
	static const uint8_t dmem_table[16][4];
	uint8_t m_handshake, m_irq;
	bool m_z8000_halt, m_z8000_mem, m_timer_irq;

	DECLARE_WRITE_LINE_MEMBER(mo_w);
	DECLARE_WRITE_LINE_MEMBER(timer_irq_w);
	IRQ_CALLBACK_MEMBER(int_cb);
};

DECLARE_DEVICE_TYPE(M24_Z8000, m24_z8000_device)

#endif // MAME_MACHINE_M24_Z8000_H
