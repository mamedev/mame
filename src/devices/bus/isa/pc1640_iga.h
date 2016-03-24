// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Amstrad PC1640 Integrated Graphics Adapter emulation

**********************************************************************/

#pragma once

#ifndef __ISA8_PC1640_IGA__
#define __ISA8_PC1640_IGA__

#include "emu.h"
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
	isa8_pc1640_iga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
};


// device type definition
extern const device_type ISA8_PC1640_IGA;



#endif
