// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_MEGADRIVE_SK_H
#define MAME_BUS_MEGADRIVE_SK_H

#pragma once

#include "md_slot.h"


// ======================> md_rom_sk_device

class md_rom_sk_device : public device_t,
						public device_md_cart_interface
{
public:
	// construction/destruction
	md_rom_sk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	md_rom_sk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// reading and writing
	virtual DECLARE_READ16_MEMBER(read) override;
	virtual DECLARE_WRITE16_MEMBER(write) override;

private:
	required_device<md_cart_slot_device> m_exp;
};


// device type definition
DECLARE_DEVICE_TYPE(MD_ROM_SK, md_rom_sk_device)

#endif // MAME_BUS_MEGADRIVE_SK_H
