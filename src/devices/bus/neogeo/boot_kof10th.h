// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef __NEOGEO_KOF10TH_H
#define __NEOGEO_KOF10TH_H

#include "slot.h"
#include "boot_misc.h"
#include "prot_misc.h"

// ======================> neogeo_kof10th_cart

class neogeo_kof10th_cart : public neogeo_bootleg_cart
{
public:
	// construction/destruction
	neogeo_kof10th_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT16 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual UINT16 get_helper() override;
	virtual UINT32 get_bank_base(UINT16 sel) override;
	virtual UINT32 get_special_bank() override;
	virtual DECLARE_READ16_MEMBER(addon_r) override;
	virtual DECLARE_READ16_MEMBER(protection_r) override;
	virtual DECLARE_WRITE16_MEMBER(protection_w) override;

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }

private:
	UINT8* m_fixed;
	UINT32 m_special_bank;
	UINT16 m_cart_ram[0x1000];
	UINT16 m_cart_ram2[0x10000];
};

// device type definition
extern const device_type NEOGEO_KOF10TH_CART;



#endif
