// license:BSD-3-Clause
// copyright-holders: Angelo Salese
#ifndef MAME_BUS_PC98_CBUS_MIF201_H
#define MAME_BUS_PC98_CBUS_MIF201_H

#pragma once

#include "slot.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mpu_pc98_device

class mif201_device : public device_t
					, public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	mif201_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	required_device_array<i8251_device, 2> m_uart;
	required_device<pit8253_device>  m_pit;

	void io_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(MIF201, mif201_device)

#endif // MAME_BUS_PC98_CBUS_MIF201_H
