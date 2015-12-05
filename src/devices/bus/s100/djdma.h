// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs Disk Jockey/DMA floppy controller board emulation

**********************************************************************/

#pragma once

#ifndef __S100_DJDMA__
#define __S100_DJDMA__

#include "emu.h"
#include "s100.h"
#include "cpu/z80/z80.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_djdma_device

class s100_djdma_device : public device_t,
							public device_s100_card_interface
{
public:
	// construction/destruction
	s100_djdma_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
};


// device type definition
extern const device_type S100_DJDMA;


#endif
