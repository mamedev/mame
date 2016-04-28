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
	neogeo_sbp_cart(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_WRITE16_MEMBER(protection_w) override;
	virtual DECLARE_READ16_MEMBER(protection_r) override;
	
	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type(void) override { return 0; }

	void patch(UINT8* cpurom, UINT32 cpurom_size);
};

extern const device_type NEOGEO_SBP_CART;


#endif
