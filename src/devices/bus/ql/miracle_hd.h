// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Miracle Hard Disk emulation

**********************************************************************/

#ifndef MAME_BUS_QL_MIRACLE_HD
#define MAME_BUS_QL_MIRACLE_HD

#pragma once

#include "rom.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> miracle_hard_disk_device

class miracle_hard_disk_device : public device_t,
							public device_ql_rom_cartridge_card_interface
{
public:
	// construction/destruction
	miracle_hard_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_ql_rom_cartridge_card_interface overrides
	virtual uint8_t read(offs_t offset, uint8_t data) override;
	virtual void write(offs_t offset, uint8_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(MIRACLE_HARD_DISK, miracle_hard_disk_device)

#endif // MAME_BUS_QL_MIRACLE_HD
