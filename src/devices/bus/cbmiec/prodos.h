// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VTS Data Professional-DOS

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_PRODOS_H
#define MAME_BUS_CBMIEC_PRODOS_H

#pragma once

#include "c1541.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c1541_professional_dos_v1_device

class c1541_professional_dos_v1_device :  public c1541_device_base
{
public:
	// construction/destruction
	c1541_professional_dos_v1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void c1541pd_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C1541_PROFESSIONAL_DOS_V1, c1541_professional_dos_v1_device)


#endif // MAME_BUS_CBMIEC_PRODOS_H
