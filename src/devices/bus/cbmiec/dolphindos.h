// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Evesham Micros Dolphin-DOS 2.0

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_DOLPHINDOS_H
#define MAME_BUS_CBMIEC_DOLPHINDOS_H

#pragma once

#include "c1541.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_dolphin_dos_device

class c1541_dolphin_dos_device : public c1541_device_base
{
public:
	// construction/destruction
	c1541_dolphin_dos_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void c1541dd_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_DOLPHIN_DOS, c1541_dolphin_dos_device)


#endif // MAME_BUS_CBMIEC_DOLPHINDOS_H
