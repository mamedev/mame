// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_FDD_2D_H
#define MAME_BUS_PC98_CBUS_FDD_2D_H

#pragma once

#include "slot.h"
#include "bus/nec_fdd/pc80s31k.h"

class fdd_2d_bridge_device : public device_t
{
public:
	fdd_2d_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<pc98_cbus_slot_device> m_bus;
	required_device<pc80s31k_device> m_fdd_if;

	void io_map(address_map &map);
};

// device type definition
DECLARE_DEVICE_TYPE(FDD_2D_BRIDGE, fdd_2d_bridge_device)

#endif // MAME_BUS_PC98_CBUS_FDD_2D_H
