// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs Disk Jockey/DMA floppy controller board emulation

**********************************************************************/

#ifndef MAME_BUS_S100_DJDMA_H
#define MAME_BUS_S100_DJDMA_H

#pragma once

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
	s100_djdma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	void djdma_io(address_map &map);
	void djdma_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(S100_DJDMA, s100_djdma_device)

#endif // MAME_BUS_S100_DJDMA_H
