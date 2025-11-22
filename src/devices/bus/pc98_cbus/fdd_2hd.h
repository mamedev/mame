// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_FDD_2HD_H
#define MAME_BUS_PC98_CBUS_FDD_2HD_H

#pragma once

#include "slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mpu_pc98_device

class fdd_2hd_bridge_device : public device_t
							, public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	fdd_2hd_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	void io_map(address_map &map) ATTR_COLD;
	required_memory_region m_bios;
};


// device type definition
DECLARE_DEVICE_TYPE(FDD_2HD_BRIDGE, fdd_2hd_bridge_device)

#endif // MAME_BUS_PC98_CBUS_FDD_2HD_H
