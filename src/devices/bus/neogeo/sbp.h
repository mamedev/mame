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
	neogeo_sbp_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual DECLARE_WRITE16_MEMBER(protection_w) override;
	virtual DECLARE_READ16_MEMBER(protection_r) override;

	virtual void decrypt_all(DECRYPT_ALL_PARAMS) override;
	virtual int get_fixed_bank_type() override { return 0; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	void patch(u8* cpurom, u32 cpurom_size);
};

DECLARE_DEVICE_TYPE(NEOGEO_SBP_CART, neogeo_sbp_cart_device)


#endif // MAME_BUS_NEOGEO_SBP_H
