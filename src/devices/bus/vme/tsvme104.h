// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_TSVME104_H
#define MAME_BUS_VME_TSVME104_H

#pragma once

#include "bus/vme/vme.h"

#include "cpu/m68000/m68010.h"
#include "machine/mc68681.h"
#include "machine/ncr5385.h"
#include "machine/68230pit.h"
#include "machine/6840ptm.h"

#include "bus/rs232/rs232.h"

class vme_tsvme104_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_tsvme104_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem(address_map &map) ATTR_COLD;
	void cpu_int(address_map &map) ATTR_COLD;

	required_device<m68010_device> m_cpu;

	required_device<ptm6840_device> m_ptm;
	required_device_array<mc68681_device, 2> m_duart;
	required_device_array<rs232_port_device, 4> m_serial;

	memory_view m_boot;
	memory_passthrough_handler m_boot_tap;
};

DECLARE_DEVICE_TYPE(VME_TSVME104, vme_tsvme104_card_device)

#endif // MAME_BUS_VME_TSVME104_H
