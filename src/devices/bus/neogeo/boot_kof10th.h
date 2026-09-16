// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_BOOT_KOF10TH_H
#define MAME_BUS_NEOGEO_BOOT_KOF10TH_H

#pragma once

#include "slot.h"
#include "boot_misc.h"
#include "prot_misc.h"

// ======================> neogeo_kof10th_cart_device

class neogeo_kof10th_cart_device : public neogeo_bootleg_cart_device
{
public:
	// construction/destruction
	neogeo_kof10th_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint16_t clock);

	// reading and writing
	virtual uint16_t get_helper() override;
	virtual uint32_t get_bank_base(uint16_t sel) override;
	virtual uint32_t get_special_bank() override;
	virtual uint16_t addon_r(offs_t offset) override;
	virtual uint16_t protection_r(address_space &space, offs_t offset) override;
	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint8_t* m_fixed;
	uint32_t m_special_bank;
	uint16_t m_cart_ram[0x1000];
	uint16_t m_cart_ram2[0x10000];
};

// device type definition
DECLARE_DEVICE_TYPE(NEOGEO_KOF10TH_CART, neogeo_kof10th_cart_device)

#endif // MAME_BUS_NEOGEO_BOOT_KOF10TH_H
