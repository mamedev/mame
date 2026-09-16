// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_PASOPIA_ROMPAC2_H
#define MAME_BUS_PASOPIA_ROMPAC2_H

#pragma once

#include "pac2.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pasopia_pa7246_device

class pasopia_pa7246_device : public device_t, public pac2_card_interface
{
public:
	// device type constructor
	pasopia_pa7246_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// pac2_card_interface overrides
	virtual u8 pac2_read(offs_t offset) override;
	virtual void pac2_write(offs_t offset, u8 data) override;

private:
	// object finders
	required_region_ptr<u8> m_kanji_rom;

	// internal state
	u32 m_kanji_index;
};

// device type declaration
DECLARE_DEVICE_TYPE(PASOPIA_PA7246, pasopia_pa7246_device)

#endif // MAME_BUS_PASOPIA_ROMPAC2_H
