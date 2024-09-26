// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_ISA_SSI2001_H
#define MAME_BUS_ISA_SSI2001_H

#pragma once

#include "isa.h"
#include "sound/mos6581.h"
#include "bus/pc_joy/pc_joy.h"

//*********************************************************************
//   TYPE DEFINITIONS
//*********************************************************************

// ====================> ssi2001_device

class ssi2001_device : public device_t,
						public device_isa8_card_interface
{
public:
	// construction/destruction
	ssi2001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<pc_joy_device> m_joy;
	required_device<mos6581_device> m_sid;
};

// device type definition

DECLARE_DEVICE_TYPE(ISA8_SSI2001, ssi2001_device)

#endif // MAME_BUS_ISA_SSI2001_H
