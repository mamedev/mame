// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_BUS_PSX_GAMEBOOSTER_H
#define MAME_BUS_PSX_GAMEBOOSTER_H

#pragma once

#include "parallel.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psx_gamebooster_device

class psx_gamebooster_device :
	public device_t,
	public psx_parallel_interface,
	public device_memory_interface
{
public:
	// construction/destruction
	psx_gamebooster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// psx_parallel_interface implementation
	virtual uint16_t exp_r(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void exp_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

private:
	required_region_ptr<u16> m_rom;

	const address_space_config m_cart_config;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_cart_space;
};


// device type declaration
DECLARE_DEVICE_TYPE(PSX_GAMEBOOSTER, psx_gamebooster_device)

#endif // MAME_BUS_PSX_GAMEBOOSTER_H
