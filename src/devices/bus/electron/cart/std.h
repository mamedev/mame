// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Acorn Electron standard cartridge emulation

***************************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_STD_H
#define MAME_BUS_ELECTRON_CART_STD_H

#pragma once

#include "slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> electron_stdcart_device

class electron_stdcart_device : public device_t,
								public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_stdcart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
};

// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_STDCART, electron_stdcart_device)

#endif // MAME_BUS_ELECTRON_CART_STD_H
