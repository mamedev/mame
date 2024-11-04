// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#ifndef MAME_BUS_NEOGEO_SBP_H
#define MAME_BUS_NEOGEO_SBP_H

#pragma once

#include "slot.h"
#include "rom.h"

/*************************************************
 sbp
 **************************************************/

class neogeo_sbp_cart_device : public neogeo_rom_device
{
public:
	neogeo_sbp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual uint16_t protection_r(address_space &space, offs_t offset) override;

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void patch(uint8_t* cpurom, uint32_t cpurom_size);
};

DECLARE_DEVICE_TYPE(NEOGEO_SBP_CART, neogeo_sbp_cart_device)


#endif // MAME_BUS_NEOGEO_SBP_H
