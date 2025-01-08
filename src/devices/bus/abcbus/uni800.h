// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_BUS_ABCBUS_UNI800_H
#define MAME_BUS_ABCBUS_UNI800_H

#pragma once

#include "abcbus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_uni800_device

class abc_uni800_device :  public device_t,
							public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_uni800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC_UNI800, abc_uni800_device)

#endif // MAME_BUS_ABCBUS_UNI800_H
