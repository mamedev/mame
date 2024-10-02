// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Clwyd Technics Colour Palette

    Micro User Chameleon (DIY) - Micro User Jan/Feb 1990

**********************************************************************/


#ifndef MAME_BUS_BBC_USERPORT_PALEXT_H
#define MAME_BUS_BBC_USERPORT_PALEXT_H

#pragma once

#include "userport.h"
#include "emupal.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_palext_device :
	public device_t,
	public device_bbc_userport_interface
{
protected:
	// construction/destruction
	bbc_palext_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	required_device<palette_device> m_palette;

	uint8_t m_colour;
	rgb_t m_palette_ram[16];
};


// ======================> bbc_chameleon_device

class bbc_chameleon_device : public bbc_palext_device
{
public:
	// construction/destruction
	bbc_chameleon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry* device_rom_region() const override;

	virtual void pb_w(uint8_t data) override;
};


// ======================> bbc_cpalette_device

class bbc_cpalette_device : public bbc_palext_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::PALETTE; }

	// construction/destruction
	bbc_cpalette_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void pb_w(uint8_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_CHAMELEON, bbc_chameleon_device)
DECLARE_DEVICE_TYPE(BBC_CPALETTE, bbc_cpalette_device)


#endif // MAME_BUS_BBC_USERPORT_PALEXT_H
