// license:BSD-3-Clause
// copyright-holders:R. Belmont,Kevin Horton
#ifndef MAME_BUS_CBUS_MPU401_H
#define MAME_BUS_CBUS_MPU401_H

#pragma once

#include "bus/cbus/pc9801_cbus.h"
#include "machine/mpu401.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_mpu401_device

class mpu_pc98_device : public device_t
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

private:
	// called back by the MPU401 core to set the IRQ line state
	void mpu_irq_out(int state);

	void map(address_map &map) ATTR_COLD;

	required_device<pc9801_slot_device> m_bus;
	required_device<mpu401_device> m_mpu401;
};


// device type definition
DECLARE_DEVICE_TYPE(MPU_PC98, mpu_pc98_device)

#endif // MAME_BUS_CBUS_MPU401_H
