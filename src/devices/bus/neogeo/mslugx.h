// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_MSLUGX_H
#define MAME_BUS_NEOGEO_MSLUGX_H

#pragma once

#include "slot.h"
#include "rom.h"
#include "prot_mslugx.h"

// ======================> neogeo_mslugx_cart_device

class neogeo_mslugx_cart_device : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_mslugx_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_prot->protection_r(space, offset, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(protection_w) override { m_prot->protection_w(space, offset, data, mem_mask); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

	required_device<mslugx_prot_device> m_prot;
};


// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_MSLUGX_CART, neogeo_mslugx_cart_device)

#endif // MAME_BUS_NEOGEO_MSLUGX_H
