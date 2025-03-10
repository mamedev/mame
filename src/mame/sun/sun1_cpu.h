// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Patrick Mackinlay

#ifndef MAME_SUN_SUN1_CPU_H
#define MAME_SUN_SUN1_CPU_H

#pragma once

#include "sun1_mmu.h"

#include "bus/multibus/multibus.h"
#include "cpu/m68000/m68000.h"
#include "machine/am9513.h"
#include "machine/z80sio.h"

class sun1cpu_device
	: public device_t
	, public device_multibus_interface
	, public m68000_device::mmu
{
public:
	sun1cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void cpu_mem(address_map &map);

	void prom0_w(u16 data);
	u16 parallel_r() { return 0x1fff; }

	void watchdog_w(int state);
	template <unsigned N> void irq_w(int state);

	// m68000_device::mmu boot mode implementation
	virtual u16 read_program(offs_t logical, u16 mem_mask) override { return m_cpu_mem.read_word(logical, mem_mask); }
	virtual void write_program(offs_t logical, u16 data, u16 mem_mask) override { m_cpu_mem.write_word(logical, data, mem_mask); }
	virtual u16 read_data(offs_t logical, u16 mem_mask) override { return m_cpu_mem.read_word(logical, mem_mask); }
	virtual void write_data(offs_t logical, u16 data, u16 mem_mask) override { m_cpu_mem.write_word(logical, data, mem_mask); }
	virtual u16 read_cpu(offs_t logical, u16 mem_mask) override { return m_cpu_spc.read_word(logical, mem_mask); }
	virtual void set_super(bool super) override {}

	bool boot() const { return m_boot.entry().has_value(); }

private:
	required_device<m68000_device> m_cpu;
	required_device<sun1mmu_device> m_mmu;
	required_device<upd7201_device> m_duart;
	required_device<am9513_device> m_stc;

	memory_view m_boot;

	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_cpu_mem;
	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_cpu_spc;

	u8 m_irq;
	bool m_watchdog;
	bool m_parity;
};

DECLARE_DEVICE_TYPE(MULTIBUS_SUN1CPU, sun1cpu_device)

#endif // MAME_SUN_SUN1_CPU_H
