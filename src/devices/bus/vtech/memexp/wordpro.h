// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Dick Smith VZ-300 WordPro Cartridge

***************************************************************************/

#ifndef MAME_BUS_VTECH_MEMEXP_WORDPRO_H
#define MAME_BUS_VTECH_MEMEXP_WORDPRO_H

#pragma once

#include "memexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_wordpro_device

class vtech_wordpro_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_wordpro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_WORDPRO, vtech_wordpro_device)

#endif // MAME_BUS_VTECH_MEMEXP_WORDPRO_H
