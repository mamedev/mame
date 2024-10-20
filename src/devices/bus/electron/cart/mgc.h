// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Mega Games Cartridge

***************************************************************************/

#ifndef MAME_BUS_ELECTRON_CART_MGC_H
#define MAME_BUS_ELECTRON_CART_MGC_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> electron_mgc_device

class electron_mgc_device : public device_t,
								public device_electron_cart_interface
{
public:
	// construction/destruction
	electron_mgc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// electron_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	uint8_t m_page_latch;
	uint8_t m_control_latch;
};

// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_MGC, electron_mgc_device)

#endif // MAME_BUS_ELECTRON_CART_MGC_H
