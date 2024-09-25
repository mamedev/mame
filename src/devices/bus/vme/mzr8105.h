// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#ifndef MAME_BUS_VME_MZR8105_H
#define MAME_BUS_VME_MZR8105_H

#pragma once

#include "bus/vme/vme.h"

class vme_mzr8105_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_mzr8105_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void mzr8105_mem(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_maincpu;
};

DECLARE_DEVICE_TYPE(VME_MZR8105, vme_mzr8105_card_device)

#endif // MAME_BUS_VME_MZR8105_H
