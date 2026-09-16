// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_FDD_2DD_H
#define MAME_BUS_PC98_CBUS_FDD_2DD_H

#pragma once

#include "slot.h"

#include "imagedev/floppy.h"
#include "machine/upd765.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mpu_pc98_device

class fdd_2dd_bridge_device : public device_t
							, public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	fdd_2dd_bridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual u8 dack_r(int line) override;
	virtual void dack_w(int line, u8 data) override;
	virtual void eop_w(int state) override;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	required_device<upd765a_device> m_fdc;
	required_memory_region m_bios;

	void io_map(address_map &map) ATTR_COLD;

	bool fdc_drive_ready_r(upd765a_device *fdc);
	u8 ctrl_r();
	void ctrl_w(u8 data);

	u8 m_ctrl;
};


// device type definition
DECLARE_DEVICE_TYPE(FDD_2DD_BRIDGE, fdd_2dd_bridge_device)

#endif // MAME_BUS_PC98_CBUS_FDD_2DD_H
