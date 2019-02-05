// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BBC Micro MRM E00 DFS emulation

***************************************************************************/

#ifndef MAME_BUS_BBC_ROM_DFS_H
#define MAME_BUS_BBC_ROM_DFS_H

#pragma once

#include "slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_mrme00_device

class bbc_mrme00_device : public device_t,
							public device_bbc_rom_interface
{
public:
	// construction/destruction
	bbc_mrme00_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_bbc_rom_interface overrides
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_MRME00, bbc_mrme00_device)


#endif // MAME_BUS_BBC_ROM_DFS_H
