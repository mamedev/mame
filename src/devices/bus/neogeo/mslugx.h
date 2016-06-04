// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_MSLUGX_CART_H
#define __NEOGEO_MSLUGX_CART_H

#include "slot.h"
#include "rom.h"
#include "prot_mslugx.h"

// ======================> neogeo_mslugx_cart

class neogeo_mslugx_cart : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_mslugx_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_prot->protection_r(space, offset, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(protection_w) override { m_prot->protection_w(space, offset, data, mem_mask); }

	required_device<mslugx_prot_device> m_prot;

};



// device type definition
extern const device_type NEOGEO_MSLUGX_CART;


#endif
