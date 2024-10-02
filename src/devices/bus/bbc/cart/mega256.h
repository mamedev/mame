// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Solidisk Mega 256 cartridge emulation

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_Mega256.html

***************************************************************************/

#ifndef MAME_BUS_BBC_CART_MEGA256_H
#define MAME_BUS_BBC_CART_MEGA256_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_mega256_device

class bbc_mega256_device : public device_t, public device_bbc_cart_interface
{
public:
	// construction/destruction
	bbc_mega256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// bbc_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	uint8_t m_page;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_MEGA256, bbc_mega256_device)

#endif // MAME_BUS_BBC_CART_MEGA256_H
