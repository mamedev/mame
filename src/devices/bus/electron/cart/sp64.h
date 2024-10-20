// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Slogger Stop Press 64 cartridge emulation

***************************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_SP64_H
#define MAME_BUS_ELECTRON_CART_SP64_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> electron_sp64_device

class electron_sp64_device : public device_t,
								public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_sp64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	uint8_t m_page_register;
};

// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_SP64, electron_sp64_device)

#endif // MAME_BUS_ELECTRON_CART_SP64_H
