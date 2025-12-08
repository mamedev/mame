// license:BSD-3-Clause
// copyright-holders:R. Belmont,Kevin Horton
#ifndef MAME_BUS_PC98_CBUS_MPU_PC98_H
#define MAME_BUS_PC98_CBUS_MPU_PC98_H

#pragma once

#include "slot.h"
#include "machine/mpu401.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mpu_pc98_device

class mpu_pc98_device : public device_t
					  , public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	mpu_pc98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	// called back by the MPU401 core to set the IRQ line state
	void mpu_irq_out(int state);

	void io_map(address_map &map) ATTR_COLD;

	required_device<mpu401_device> m_mpu401;
};


// device type definition
DECLARE_DEVICE_TYPE(MPU_PC98, mpu_pc98_device)

#endif // MAME_BUS_PC98_CBUS_MPU_PC98_H
