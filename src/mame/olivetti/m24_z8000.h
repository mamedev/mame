// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_OLIVETTI_M24_Z8000_H
#define MAME_OLIVETTI_M24_Z8000_H

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

	uint16_t pmem_r(offs_t offset, uint16_t mem_mask = ~0);
	void pmem_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dmem_r(offs_t offset, uint16_t mem_mask = ~0);
	void dmem_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t i86_io_r(offs_t offset, uint16_t mem_mask = ~0);
	void i86_io_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void irqctl_w(uint8_t data);
	void serctl_w(uint8_t data);
	uint8_t handshake_r();
	void handshake_w(uint8_t data);

	void halt_w(int state) { m_z8000->set_input_line(INPUT_LINE_HALT, state); }
	void int_w(int state) { m_z8000->set_input_line(z8001_device::VI_LINE, state); }

	bool halted() const { return m_z8000_halt; }

	void z8000_data(address_map &map) ATTR_COLD;
	void z8000_io(address_map &map) ATTR_COLD;
	void z8000_prog(address_map &map) ATTR_COLD;
protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<z8001_device> m_z8000;
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	devcb_write_line m_halt_out;
	static const uint8_t pmem_table[16][4];
	static const uint8_t dmem_table[16][4];
	uint8_t m_handshake, m_irq;
	bool m_z8000_halt, m_z8000_mem, m_timer_irq;

	void mo_w(int state);
	void timer_irq_w(int state);
	uint16_t nviack_r();
	uint16_t viack_r();
};

DECLARE_DEVICE_TYPE(M24_Z8000, m24_z8000_device)

#endif // MAME_OLIVETTI_M24_Z8000_H
