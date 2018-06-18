// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_BUS_PSX_GAMEBOOSTER_H
#define MAME_BUS_PSX_GAMEBOOSTER_H

#pragma once


#include "parallel.h"
#include "bus/gameboy/rom.h"
#include "bus/gameboy/mbc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psx_gamebooster_device

class psx_gamebooster_device :
	public device_t,
	public psx_parallel_interface
{
public:
	// construction/destruction
	psx_gamebooster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual DECLARE_READ16_MEMBER(exp_r) override;
	virtual DECLARE_WRITE16_MEMBER(exp_w) override;

private:
	required_memory_region m_rom;
	required_device<gb_cart_slot_device> m_cartslot;
};


// device type definition
DECLARE_DEVICE_TYPE(PSX_GAMEBOOSTER, psx_gamebooster_device)

#endif // MAME_BUS_PSX_GAMEBOOSTER_H
