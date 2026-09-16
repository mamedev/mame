// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_MULTIBUS_CPUAP_H
#define MAME_BUS_MULTIBUS_CPUAP_H

#pragma once

#include "multibus.h"

#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"
#include "machine/mc146818.h"


class cpuap_device
	: public device_t
	, public device_multibus_interface
{
public:
	cpuap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;

	void bus_timeout(u8 data);
	void bus_mem_w(offs_t offset, u16 data, u16 mem_mask) { m_bus->space(AS_PROGRAM).write_word(0xe00000 | offset << 1, data, mem_mask); }
	u16 bus_mem_r(offs_t offset, u16 mem_mask) { return m_bus->space(AS_PROGRAM).read_word(0xe00000 | offset << 1, mem_mask); }
	void bus_pio_w(offs_t offset, u16 data, u16 mem_mask) { m_bus->space(AS_IO).write_word(offset << 1, data, mem_mask); }
	u16 bus_pio_r(offs_t offset, u16 mem_mask) { return m_bus->space(AS_IO).read_word(offset << 1, mem_mask); }

	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;
	required_device<mc146818_device> m_rtc;
	required_ioport m_s7;
	required_ioport m_s8;
	memory_view m_boot;

	u8 m_prdia;
	u8 m_poff;
	u8 m_nmi;
};

DECLARE_DEVICE_TYPE(CPUAP, cpuap_device)

#endif // MAME_BUS_MULTIBUS_CPUAP_H
