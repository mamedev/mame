// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Amstrad PC1640 Integrated Graphics Adapter emulation

**********************************************************************/

#ifndef MAME_BUS_ISA_PC1640_IGA_H
#define MAME_BUS_ISA_PC1640_IGA_H

#pragma once

#include "isa.h"
#include "ega.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_pc1640_iga_device

class isa8_pc1640_iga_device : public isa8_ega_device
{
public:
	// construction/destruction
	isa8_pc1640_iga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_PC1640_IGA, isa8_pc1640_iga_device)


#endif // MAME_BUS_ISA_PC1640_IGA_H
