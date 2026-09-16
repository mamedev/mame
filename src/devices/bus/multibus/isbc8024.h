// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_MULTIBUS_ISBC8024_H
#define MAME_BUS_MULTIBUS_ISBC8024_H

#pragma once

#include "multibus.h"

#include "cpu/i8085/i8085.h"
#include "machine/pit8253.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"

#include "bus/rs232/rs232.h"

class isbc8024_device
	: public device_t
	, public device_multibus_interface
{
public:
	isbc8024_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem_map(address_map &map) ATTR_COLD;
	void cpu_pio_map(address_map &map) ATTR_COLD;

	void bus_mem_w(offs_t offset, u8 data) { m_bus->space(AS_PROGRAM).write_byte(offset, data); }
	u8 bus_mem_r(offs_t offset) { return m_bus->space(AS_PROGRAM).read_byte(offset); }
	void bus_pio_w(offs_t offset, u8 data) { m_bus->space(AS_IO).write_byte(offset, data); }
	u8 bus_pio_r(offs_t offset) { return m_bus->space(AS_IO).read_byte(offset); }

	required_device<i8085a_cpu_device> m_cpu;
	required_device<pit8254_device> m_pit;
	required_device<i8251_device> m_pci;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<pic8259_device> m_pic;
	required_device<rs232_port_device> m_j3;

	required_ioport m_conf;

	bool m_installed;
};

DECLARE_DEVICE_TYPE(ISBC8024, isbc8024_device)

#endif // MAME_BUS_MULTIBUS_ISBC8024_H
