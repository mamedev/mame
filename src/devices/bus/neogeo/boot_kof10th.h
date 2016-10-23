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
	neogeo_kof10th_cart(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// reading and writing
	virtual uint16_t get_helper() override;
	virtual uint32_t get_bank_base(uint16_t sel) override;
	virtual uint32_t get_special_bank() override;
	virtual uint16_t addon_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual uint16_t protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual void protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override;

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }

private:
	uint8_t* m_fixed;
	uint32_t m_special_bank;
	uint16_t m_cart_ram[0x1000];
	uint16_t m_cart_ram2[0x10000];
};

// device type definition
extern const device_type NEOGEO_KOF10TH_CART;



#endif
