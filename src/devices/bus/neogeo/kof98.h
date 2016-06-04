// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_KOF98_H
#define __NEOGEO_KOF98_H

#include "slot.h"
#include "rom.h"
#include "prot_kof98.h"

// ======================> neogeo_kof98_cart

class neogeo_kof98_cart : public neogeo_rom_device
{
public:
	// construction/destruction
	neogeo_kof98_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(protection_r) override { return m_prot->protection_r(space, offset, mem_mask); }
	virtual DECLARE_WRITE16_MEMBER(protection_w) override { m_prot->protection_w(space, offset, data, mem_mask); }

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;

	required_device<kof98_prot_device> m_prot;

};



// device type definition
extern const device_type NEOGEO_KOF98_CART;


#endif
