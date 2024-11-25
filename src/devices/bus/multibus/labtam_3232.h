// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_MULTIBUS_LABTAM_3232_H
#define MAME_BUS_MULTIBUS_LABTAM_3232_H

#pragma once

#include "multibus.h"

#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"

class labtam_3232_device
	: public device_t
	, public device_multibus_interface
{
public:
	labtam_3232_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;

	//void bus_mem_w(offs_t offset, u16 data, u16 mem_mask) { m_bus->space(AS_PROGRAM).write_word(0xe00000 | offset << 1, data, mem_mask); }
	//u16 bus_mem_r(offs_t offset, u16 mem_mask) { return m_bus->space(AS_PROGRAM).read_word(0xe00000 | offset << 1, mem_mask); }
	//void bus_pio_w(offs_t offset, u16 data, u16 mem_mask) { m_bus->space(AS_IO).write_word(offset << 1, data, mem_mask); }
	//u16 bus_pio_r(offs_t offset, u16 mem_mask) { return m_bus->space(AS_IO).read_word(offset << 1, mem_mask); }

	required_device<ns32032_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;

	memory_view m_boot;

	bool m_installed;
};

DECLARE_DEVICE_TYPE(LABTAM_3232, labtam_3232_device)

#endif // MAME_BUS_MULTIBUS_LABTAM_3232_H
