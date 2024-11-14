// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_SMVME2000_H
#define MAME_BUS_VME_SMVME2000_H

#pragma once

#include "bus/vme/vme.h"

#include "cpu/m68000/m68000.h"
//#include "machine/m68451.h"
#include "machine/mc68681.h"
#include "machine/68230pit.h"

#include "bus/rs232/rs232.h"

class vme_smvme2000_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_smvme2000_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem(address_map &map) ATTR_COLD;
	void cpu_int(address_map &map) ATTR_COLD;

	required_device<m68000_device> m_cpu;
	//optional_device<m68451_device> m_mmu;
	required_device<scn2681_device> m_duart;
	required_device<pit68230_device> m_pit;

	required_device_array<rs232_port_device, 2> m_serial;

	output_finder<> m_fail;

	required_region_ptr<u16> m_eprom;
	required_shared_ptr<u16> m_ram;

	memory_passthrough_handler m_boot;
};

DECLARE_DEVICE_TYPE(VME_SMVME2000, vme_smvme2000_card_device)

#endif // MAME_BUS_VME_SMVME2000_H
