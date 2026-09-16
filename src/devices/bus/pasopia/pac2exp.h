// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_PASOPIA_PAC2EXP_H
#define MAME_BUS_PASOPIA_PAC2EXP_H

#pragma once

#include "pac2.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pasopia_pa7234_device

class pasopia_pa7234_device : public device_t, public pac2_card_interface
{
public:
	// device type constructor
	pasopia_pa7234_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// pac2_card_interface overrides
	virtual u8 pac2_read(offs_t offset) override;
	virtual void pac2_write(offs_t offset, u8 data) override;

private:
	// object finders
	required_device_array<pasopia_pac2_slot_device, 4> m_slot;

	// internal state
	u8 m_slot_selected;
};

// device type declaration
DECLARE_DEVICE_TYPE(PASOPIA_PA7234, pasopia_pa7234_device)

#endif // MAME_BUS_PASOPIA_PAC2EXP_H
