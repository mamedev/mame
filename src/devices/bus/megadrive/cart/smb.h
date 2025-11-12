// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_SMB_H
#define MAME_BUS_MEGADRIVE_CART_SMB_H

#pragma once

#include "rom.h"
#include "slot.h"

class megadrive_unl_smb_device : public megadrive_rom_device
{
public:
	megadrive_unl_smb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void time_io_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_smb2_device : public megadrive_rom_device
{
public:
	megadrive_unl_smb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void time_io_map(address_map &map) override ATTR_COLD;
};

class megadrive_unl_rockmanx3_device : public megadrive_rom_device
{
public:
	megadrive_unl_rockmanx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;
};



DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_SMB,        megadrive_unl_smb_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_SMB2,       megadrive_unl_smb2_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_UNL_ROCKMANX3,  megadrive_unl_rockmanx3_device)


#endif // MAME_BUS_MEGADRIVE_CART_SMB_H
