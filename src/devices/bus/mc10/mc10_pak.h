// license:BSD-3-Clause
// copyright-holders:tim lindner
#ifndef MAME_BUS_MC10_MC10_PAK_H
#define MAME_BUS_MC10_MC10_PAK_H

#pragma once

#include "mc10cart.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mc10_pak_device

class mc10_pak_device :
		public device_t,
		public device_mc10cart_interface
{
public:
	// construction/destruction
	mc10_pak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual memory_region *get_cart_memregion() override;

protected:
	mc10_pak_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;

	// internal state
	required_memory_region m_eprom;
};

// device type definitions
DECLARE_DEVICE_TYPE(MC10_PAK, mc10_pak_device)

#endif // MAME_BUS_MC10_COCO_PAK_H
