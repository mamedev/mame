// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli
#pragma once

#ifndef __NEOGEO_BOOTSBP_H
#define __NEOGEO_BOOTSBP_H

#include "slot.h"
#include "rom.h"

/*************************************************
 sbp
 **************************************************/

class neogeo_sbp_cart : public neogeo_rom_device
{
public:
	neogeo_sbp_cart(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override;
	virtual uint16_t protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }

	void patch(uint8_t* cpurom, uint32_t cpurom_size);
};

extern const device_type NEOGEO_SBP_CART;


#endif
